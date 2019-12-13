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
    f:write("min,max,mean,stddev,median,90th,99th,duration,requests,bytes\n")
    f:write(string.format("%f,%f,%f,%f,%f,%f,%f,%d,%d,%d\n",
        latency.min,
        latency.max,
        latency.mean,
        latency.stdev,
        latency:percentile(50),
        latency:percentile(90),
        latency:percentile(99),
        summary["duration"],
        summary["requests"],
        summary["bytes"]
    ))

    f:close()
end
