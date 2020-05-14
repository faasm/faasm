#include <algorithm>
#include <util/macros.h>
#include <util/logging.h>
#include <sys/socket.h>
#include "TCPMessage.h"

#define RECV_BUF_SIZE 256

namespace tcp {
    void freeTcpMessage(TCPMessage *msg) {
        if(msg != nullptr) {
            delete[] msg->buffer;
            delete msg;
        }
    }

    size_t tcpMessageLen(TCPMessage *msg) {
        return sizeof(TCPMessage) + msg->len;
    }
    
    uint8_t *tcpMessageToBuffer(TCPMessage *msg) {
        // Overall buffer length
        size_t bufferLen = tcpMessageLen(msg);
        auto buffer = new uint8_t[bufferLen];

        // Size of struct
        size_t structSize = sizeof(TCPMessage);

        // Copy struct in
        auto bytes = BYTES(msg);
        std::copy(bytes, bytes + structSize, buffer);

        // Copy data buffer after
        std::copy(msg->buffer, msg->buffer + msg->len, buffer + structSize);

        return buffer;
    }

    TCPMessage *tcpMessageFromBuffer(uint8_t* buffer) {
        auto tcpMessage = reinterpret_cast<TCPMessage *>(buffer);
        tcpMessage->buffer = buffer + sizeof(TCPMessage);

        return tcpMessage;
    }

    TCPMessage *receiveTcpMessage(int fd) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Set up TCP message to hold data
        auto msg = new TCPMessage();
        int packetCount = 0;
        size_t bytesReceived = 0;

        while (true) {
            // Receive the data
            uint8_t buffer[RECV_BUF_SIZE];
            int nRecv = ::recv(fd, buffer, RECV_BUF_SIZE, 0);

            if (nRecv <= 0) {
                // Connection closed
                if (nRecv == 0) {
                    logger->trace("[TCP] - other end closed {}", fd);
                    throw TCPSenderClosedException("Other end closed");
                }

                // Error
                if (nRecv < 0) {
                    if (errno != EWOULDBLOCK) {
                        logger->error("Failed to recv. Errno {} ({})", errno, strerror(errno));
                        throw TCPFailedException("Recv error");
                    }

                    logger->trace("[TCP] - poll would block for {}", fd);
                    break;
                }
            } else {
                uint8_t *bufferStart;

                if (packetCount == 0) {
                    // First packet contains the main struct
                    size_t messageHeaderSize = sizeof(TCPMessage);
                    std::copy(buffer, buffer + messageHeaderSize, BYTES(msg));

                    // Provision the message buffer
                    if (msg->len > 0) {
                        msg->buffer = new uint8_t[msg->len];
                    }

                    // Offset the rest of the buffer
                    bufferStart = buffer + messageHeaderSize;
                    nRecv -= messageHeaderSize;
                } else {
                    bufferStart = buffer;
                }

                // Insert the data from this packet
                if (nRecv > 0) {
                    uint8_t *msgBufferPosition = msg->buffer + bytesReceived;
                    std::copy(bufferStart, bufferStart + nRecv, msgBufferPosition);
                }

                bytesReceived += nRecv;

                if (bytesReceived >= msg->len) {
                    logger->trace("[TCP] - finished message type {} ({} across {} packets) from {}",
                                  msg->type, bytesReceived, packetCount + 1, fd);
                    break;
                } else {
                    logger->trace("[TCP] - processed packet {} of {} bytes from {}", packetCount, nRecv,
                                  fd);
                    packetCount++;
                }
            }
        }

        // Check we've received what we expected
        if (bytesReceived != msg->len) {
            logger->warn("[TCP] - did not receive exactly the bytes (expected {}, got {})", msg->len,
                         bytesReceived);
        }

        return msg;
    }
}