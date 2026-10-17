// Loads picks.json (written every morning by the C++ program) and shows it on the page.
// picks.json = { record: {...totals...}, picks: [newest first, ...] }

function show(id, text) {
  document.getElementById(id).textContent = text;
}

function formatDate(isoDate) {
  // "2026-10-15" -> "Thu, Oct 15". Adding T12:00 avoids time-zone off-by-one days.
  const date = new Date(isoDate + "T12:00:00");
  return date.toLocaleDateString("en-US", { weekday: "short", month: "short", day: "numeric" });
}

// 12.5 -> "+$12.50", -10 -> "−$10.00"
function formatMoney(amount) {
  const sign = amount > 0 ? "+" : amount < 0 ? "−" : "";
  return sign + "$" + Math.abs(amount).toFixed(2);
}

// {wins: 7, losses: 3, pushes: 1} -> "7-3-1" (pushes only shown if there are any)
function formatRecord(r) {
  return r.wins + "-" + r.losses + (r.pushes > 0 ? "-" + r.pushes : "");
}

// Same formula as record.cpp, so each row can show what that bet made.
function profitFor(pick, stake) {
  if (pick.result === "loss") return -stake;
  if (pick.result !== "win") return 0;
  let odds = parseInt(pick.odds, 10);
  if (!(odds >= 100 || odds <= -100)) odds = -110;
  return odds > 0 ? stake * odds / 100 : stake * 100 / -odds;
}

function renderTodaysPick(pick) {
  show("sport", pick.sport);
  show("date", formatDate(pick.date));
  show("bet", pick.bet);
  show("event", pick.event);
  show("odds", pick.odds);
  show("start", pick.start_time);
  show("confidence", pick.confidence);
  show("reasoning", pick.reasoning);

  const sources = pick.sources || [];
  if (sources.length > 0) {
    const list = document.getElementById("sources");
    for (const source of sources) {
      const link = document.createElement("a");
      link.href = source.url;
      link.textContent = source.title;
      link.target = "_blank";
      link.rel = "noopener";
      const item = document.createElement("li");
      item.appendChild(link);
      list.appendChild(item);
    }
    document.getElementById("sources-box").hidden = false;
  }
}

function renderRecord(record) {
  show("all-time", formatRecord(record.all_time));
  show("last-10", formatRecord(record.last_10));
  show("stake-label", "$" + record.stake + " per pick");

  const profit = document.getElementById("profit");
  profit.textContent = formatMoney(record.all_time.profit);
  profit.classList.add(record.all_time.profit >= 0 ? "up" : "down");
}

const RESULT_LABELS = { win: "W", loss: "L", push: "P", pending: "…" };

function renderRecentPicks(picks, stake) {
  // Skip today's pick (it's shown above) and show the 10 before it.
  const recent = picks.slice(1, 11);
  document.getElementById("recent-empty").hidden = recent.some((p) => p.result !== "pending");

  const list = document.getElementById("recent");
  for (const pick of recent) {
    const item = document.createElement("li");
    item.className = "row " + pick.result;

    const badge = document.createElement("span");
    badge.className = "badge mono";
    badge.textContent = RESULT_LABELS[pick.result] || "?";

    const text = document.createElement("span");
    text.className = "row-text";
    const bet = document.createElement("strong");
    bet.textContent = pick.bet + " (" + pick.odds + ")";
    const detail = document.createElement("small");
    detail.textContent = formatDate(pick.date) + " · " + (pick.final_score || pick.event);
    text.append(bet, detail);

    const money = document.createElement("span");
    money.className = "row-money mono";
    money.textContent = pick.result === "pending" ? "pending" : formatMoney(profitFor(pick, stake));

    item.append(badge, text, money);
    list.appendChild(item);
  }
}

// "no-store" makes the browser fetch a fresh copy, so nobody sees yesterday's pick.
fetch("picks.json", { cache: "no-store" })
  .then((response) => {
    if (!response.ok) throw new Error("HTTP " + response.status);
    return response.json();
  })
  .then((data) => {
    if (data.picks.length > 0) renderTodaysPick(data.picks[0]);
    renderRecord(data.record);
    renderRecentPicks(data.picks, data.record.stake);
  })
  .catch((error) => {
    show("sport", "Offline");
    show("bet", "No pick right now");
    show("event", "Check back later.");
    console.error("Could not load picks.json:", error);
  });
