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


## Starting the Server

You will need **two terminal windows** open.

### Terminal 1: Start the backend server
Run:

```bash
python -m uvicorn server:app --host 0.0.0.0 --port 8001
```

### Terminal 2: Start the Cloudflare tunnel
Run:
```bash
cloudflared tunnel --url http://localhost:8001
```

Make sure Cloudflared is installed irst. You can download it here:
https://developers.cloudflare.com/cloudflare-one/connections/connect-networks/downloads/

### Final step
After starting the tunnel, Cloudflared will generate a public URL.
This URL should:
- be opened in a browser to view the website
- be copied into config.h by replacing the value of BASE_URL
