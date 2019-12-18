bench_mode = os.getenv("BENCH_MODE")
delay_ms = tonumber(os.getenv("BENCH_DELAY_MS"))
cold_start_interval = os.getenv("COLD_START_INTERVAL")

print("BENCH_MODE=" .. bench_mode .. " DELAY=" .. delay_ms .. " COLD_START_INTERVAL=" .. cold_start_interval)

-- Request set-up

wrk.method = "POST"
wrk.body   = string.format('{"user": "tf", "function": "image", "cold_start_interval": "%s"}', cold_start_interval)
wrk.headers["Content-Type"] = "application/json"

-- Different headers for native/ wasm
if bench_mode == "NATIVE" then
    wrk.headers["Host"] = "faasm-image.faasm.example.com"
else
    wrk.headers["Host"] = "faasm-worker.faasm.example.com"
end

-- Delay function (must return ms)
-- To avoid bursts of requests, stagger them over this delay

function delay()
   return math.random(0, delay_ms)
end

-- Output to CSV

done = function(summary, latency, requests)
    output_file = "/tmp/wrk_results.txt"
    print("Writing output to " .. output_file)

    f = io.open(output_file, "w")
    f:write(string.format("MIN=%f\n", latency.min))
    f:write(string.format("MAX=%f\n", latency.max))
    f:write(string.format("STDDEV=%f\n", latency.stdev))
    f:write(string.format("DURATION=%d\n", summary["duration"]))
    f:write(string.format("REQUESTS=%d\n", summary["requests"]))
    f:write(string.format("MEDIAN=%d\n", latency:percentile(50)))
    f:write(string.format("90TH=%d\n", latency:percentile(90)))
    f:write(string.format("10TH=%d\n", latency:percentile(10)))

    -- Write out all latency percentiles from 0-99
    f:write(string.format("LATENCIES="))
    f:write(string.format("0"))
    for p in range(1,99) do
       f:write(string.format(",%d", latency:percentile(p)))
    end
    f:write("\n")

    f:close()
end
