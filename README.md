# whatshouldibet.com

Once a day, a C++ program asks Claude (with web search turned on) for the best
sports bet of the day. A static website shows the answer.

## How it works

```
GitHub Actions (daily, 10 AM ET)
   │
   ├─ builds + runs  daily_pick  (C++)
   │      │
   │      ├─ prompt.cpp         builds the question, including today's date
   │      ├─ claude_client.cpp  HTTPS POST to the Claude API, web search on
   │      └─ pick.cpp           pulls the JSON answer + sources out of the reply
   │      ▼
   │   site/pick.json
   │
   └─ publishes site/ to GitHub Pages
          index.html + style.css + app.js  →  reads pick.json, shows it
```

Each piece only knows its own job:

| Folder / file | Job | Knows about |
|---|---|---|
| `src/claude_client.*` | Talk to the Claude API | HTTP, API keys, JSON |
| `src/prompt.*` | Decide what to ask | Betting, the date |
| `src/pick.*` | Turn the reply into `pick.json` | The shape of the answer |
| `src/main.cpp` | Glue: ask → parse → save | Everything, briefly |
| `site/` | Display `pick.json` | HTML only, never sees the API key |
| `.github/workflows/daily-pick.yml` | Run it every day and publish | GitHub |

## One-time setup

1. **Get an API key** at https://platform.claude.com (Settings → API keys) and add a little credit.
   Each run costs a few cents (a handful of searches at $0.01 each plus tokens).
2. **Store the key in GitHub**: repo → Settings → Secrets and variables → Actions →
   New repository secret. Name it `ANTHROPIC_API_KEY`.
3. **Turn on Pages**: repo → Settings → Pages → Source: **GitHub Actions**.
4. **Run it once**: Actions tab → "Daily pick" → Run workflow. After that it runs daily.
5. **Custom domain** (after buying whatshouldibet.com): Settings → Pages → Custom domain,
   then add the DNS records GitHub shows you at your domain registrar.

## Running locally (optional)

Needs CMake, a C++17 compiler and libcurl. The easiest route on Windows is WSL (Ubuntu):

```bash
sudo apt install cmake g++ libcurl4-openssl-dev
cmake -S . -B build && cmake --build build
export ANTHROPIC_API_KEY=sk-ant-...
./build/daily_pick site/pick.json
```

To preview the site, serve the folder (opening index.html directly blocks `fetch`):

```bash
cd site && python3 -m http.server 8000   # then open http://localhost:8000
```

## Changing things

- **The question** → `src/prompt.cpp`
- **The model** → set the `CLAUDE_MODEL` environment variable (default `claude-sonnet-5-5`)
- **The time it runs** → the `cron` line in the workflow (it's in UTC)
- **The look** → `site/style.css` (colors are at the top)

For entertainment only. Must be 21+ and in a legal jurisdiction. 1-800-GAMBLER.
