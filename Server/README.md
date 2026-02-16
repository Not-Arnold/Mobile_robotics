To start the server:

Required 2 terminals.

Run:
    "python -m uvicorn server:app --host 0.0.0.0 --port 8001"
    on one terminal

then run:
    "cloudflared tunnel --url http://localhost:8001"

    Ensure that you have clourflared installed from
    "https://developers.cloudflare.com/cloudflare-one/connections/connect-networks/downloads/"

After running the cloudflared code for the tunnel, it should give you a URL, you can paste that link into your browser to see the website but also replase the URL in the code in "config.h". "String BASE_URL".
