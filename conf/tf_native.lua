wrk.method = "POST"
wrk.body   = '{"user": "tf", "function": "image"}'
wrk.headers["Content-Type"] = "application/json"
wrk.headers["Host"] = "faasm-image.faasm.example.com"

