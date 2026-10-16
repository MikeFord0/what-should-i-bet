// Loads pick.json (written once a day by the C++ program) and shows it on the page.

function show(id, text) {
  document.getElementById(id).textContent = text;
}

function formatDate(isoDate) {
  // "2026-10-15" -> "Thu, Oct 15". Adding T12:00 avoids time-zone off-by-one days.
  const date = new Date(isoDate + "T12:00:00");
  return date.toLocaleDateString("en-US", { weekday: "short", month: "short", day: "numeric" });
}

function render(pick) {
  show("sport", pick.sport);
  show("date", formatDate(pick.date));
  show("bet", pick.bet);
  show("event", pick.event);
  show("odds", pick.odds);
  show("start", pick.start_time);
  show("confidence", pick.confidence);
  show("reasoning", pick.reasoning);
  show("model", "Picked by " + pick.model);

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

// "no-store" makes the browser fetch a fresh copy, so nobody sees yesterday's pick.
fetch("pick.json", { cache: "no-store" })
  .then((response) => {
    if (!response.ok) throw new Error("HTTP " + response.status);
    return response.json();
  })
  .then(render)
  .catch((error) => {
    show("sport", "Offline");
    show("bet", "No pick right now");
    show("event", "Check back later.");
    console.error("Could not load pick.json:", error);
  });
