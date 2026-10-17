# whatshouldibet.com

Every morning, a C++ program asks Claude (with web search turned on) to grade
yesterday's pick and choose today's best sports bet. A static website shows the
pick, the win/loss record, and the profit from betting $10 on every pick.

## How it works

```
GitHub Actions (every night, 3:00 AM ET)
   │
   ├─ builds + runs  daily_pick  (C++)
   │      1. grade  pending past picks   → win / loss / push
   │      2. pick   today's bet
   │      3. tally  record + $10-per-bet profit
   │      ▼
   │   site/picks.json   (the whole history + totals)
   │
   └─ publishes site/ to GitHub Pages
          index.html + style.css + app.js  →  reads picks.json, shows it
```

Each piece only knows its own job:

| Folder / file | Job | Knows about |
|---|---|---|
| `src/claude_client.*` | Talk to the Claude API, read the JSON answer | HTTP, API keys, JSON |
| `src/prompt.*` | Decide what to ask (pick + grade) | Betting, the date |
| `src/pick.*` | Check answers have the right fields | The shape of a pick |
| `src/record.*` | Win/loss record and profit math | American odds |
| `src/history.*` | Read/write `picks.json` | The file format |
| `src/main.cpp` | Glue: grade → pick → tally → save | Everything, briefly |
| `site/` | Display `picks.json` | HTML only, never sees the API key |
| `.github/workflows/daily-pick.yml` | Run it every morning and publish | GitHub |

**Profit math** (`record.cpp`): a win at −110 pays $10 × 100/110 = $9.09; a win at
+145 pays $10 × 145/100 = $14.50; a loss is −$10; a push (refund) is $0.

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
./build/daily_pick site/picks.json
```

To preview the site, serve the folder (opening index.html directly blocks `fetch`):

```bash
cd site && python3 -m http.server 8000   # then open http://localhost:8000
```

## Changing things

- **The question** → `src/prompt.cpp`
- **The model** → set the `CLAUDE_MODEL` environment variable (default `claude-sonnet-5-5`)
- **The time it runs** → the `cron` line in the workflow (Eastern time, set by `timezone`)
- **The look** → `site/style.css` (colors are at the top)

For entertainment only. Must be 21+ and in a legal jurisdiction. 1-800-GAMBLER.
