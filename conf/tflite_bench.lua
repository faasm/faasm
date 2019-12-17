bench_mode = os.getenv("BENCH_MODE")
delay_ms = tonumber(os.getenv("BENCH_DELAY_MS"))

print("BENCH_MODE = " .. bench_mode .. " DELAY= " .. delay_ms)

-- Request set-up

wrk.method = "POST"
wrk.body   = '{"user": "tf", "function": "image"}'
wrk.headers["Content-Type"] = "application/json"

-- Different headers for native/ wasm
if bench_mode == "NATIVE" then
    wrk.headers["Host"] = "faasm-image.faasm.example.com"
else
    wrk.headers["Host"] = "faasm-worker.faasm.example.com"
end

-- Delay function (must return ms)

function delay()
   return delay_ms
end

-- Output to CSV

done = function(summary, latency, requests)
    output_file = "/tmp/wrk_results.csv"
    print("Writing output to " .. output_file)

    f = io.open(output_file, "w")
    f:write("min,max,mean,stddev,duration,requests,bytes,5th,10th,15th,20th,25th,30th,35th,40th,45th,50th,55th,60th,65th,70th,75th,80th,85th,90th,95th,100th\n")
    f:write(string.format("%f,%f,%f,%f, %d,%d,%d, %f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
        latency.min,
        latency.max,
        latency.mean,
        latency.stdev,
        summary["duration"],
        summary["requests"],
        summary["bytes"],
        latency:percentile(5),
        latency:percentile(10),
        latency:percentile(15),
        latency:percentile(20),
        latency:percentile(25),
        latency:percentile(30),
        latency:percentile(35),
        latency:percentile(40),
        latency:percentile(45),
        latency:percentile(50),
        latency:percentile(55),
        latency:percentile(60),
        latency:percentile(65),
        latency:percentile(70),
        latency:percentile(75),
        latency:percentile(80),
        latency:percentile(85),
        latency:percentile(90),
        latency:percentile(95),
        latency:percentile(99)
    ))

    f:close()
end
