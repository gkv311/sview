/* API communication */

const api = { obj: window, origin: document.location.origin };

export function api_send(o) {
    api.obj?.postMessage(o, api.origin);
}

export function api_extern(url="http://localhost:8080/") {
    const u = new URL(url);
    const win = window.open(u.href, "sview");
    if (win) {
        api.obj = win;
        api.origin = u.origin;
    }
}

setInterval(_ => {
    if (api.obj.closed)
        msgEvents["info:Online"]?.({ type: "info:Online", online: false });
}, 2000);

/* Buttons */

const actions = ["PlayPause", "ListPrev", "ListNext", "Fullscreen", "AudioMute"];
for (const action of actions) {
    const btn = document.getElementById("btn_" + action);
    btn?.addEventListener("click", e => {
        e.preventDefault();
        api_send({ type: "action:" + action })
    });
}

/* Volume bar */

document.getElementById("volume_box")?.addEventListener("click", e => {
    const w = document.getElementById("volume_box").offsetWidth;
    const percent = Math.round((e.offsetX / w) * 100);
    api_send({ type: "op:SetVolume", volume: percent });
});

document.getElementById("volume_box")?.addEventListener("dblclick", _ => {
    api_send({ type: "op:SetVolume", volume: 100 });
});

let wheel_timeout = undefined;
document.getElementById("volume_icon")?.addEventListener('wheel', e => {
    const volume = Math.round(Math.min(Math.max(0, last_volume - (e.deltaX + e.deltaY)/2), 120));
    update_volume_ui(volume);
    clearTimeout(wheel_timeout);
    wheel_timeout = setTimeout(_ => {
        api_send({ type: "op:SetVolume", volume: volume });
    }, 200);
}, { passive: true });

let last_volume = 0;
function update_volume_ui(v) {
    last_volume = v;
    document.getElementById("volume_nb").innerHTML = v + "%";
    const vol_span = document.getElementById("volume");
    if (vol_span) {
        vol_span.style.width = Math.min(Math.max(0, v), 100) + '%';
        vol_span.classList.toggle("hot", v > 100);
    }
}

/* Message reception */

let last_hl = -1;
const title0 = document.title;
const msgEvents = {
    "ready": msg => {
        if ("sview" in msg)
            api_send({ type: "hello", name: "WebUI interface" });
    },

    "info:Version": msg => {
        document.getElementById("stVer").innerText = msg.version;
    },

    "info:Online": msg => {
        document.getElementById("stOffline")?.classList.toggle("hidden", !!msg.online);
        document.getElementById("controls")?.classList.toggle("hidden", !msg.online);
        document.getElementById("playlist")?.classList.toggle("hidden", !msg.online);
    },

    "info:Title": msg => {
        const t = msg.title;
        document.getElementById("stTitle").innerText = t ? "Current: " + t : "";
        document.title = (t ? t + " - " : "") + title0;
    },

    "info:PlaylistIndex": msg => {
        const playlist = document.getElementById("playlist");
        playlist?.children.item(last_hl)?.classList.remove("hl");
        last_hl = msg.index;
        playlist?.children.item(last_hl)?.classList.add("hl");
    },

    "info:Playlist": msg => {
        const playlist = document.getElementById("playlist");
        playlist.innerText = "";
        let i = 0;
        for (const title of msg.titles) {
            const line = document.createElement('span');
            line.innerText = title;
            line.dataset["index"] = i;
            line.classList.toggle("hl", i == last_hl);
            line.onclick = e => {
                e.preventDefault();
                api_send({
                    type: "op:PlaylistPlay",
                    index: Number(line.dataset["index"]),
                });
            }
            playlist.append(line);
            i++;
        }
    },

    "info:Volume": msg => update_volume_ui(msg.volume),
}

window.addEventListener("message", e => {
    if (e.origin != api.origin) return;
    msgEvents[e.data.type]?.(e.data);
});

/* Theme */

for (const r of document.querySelectorAll('input[name="theme"]')) {
    r.onchange = _ => {
        if (!r.checked) return;
        const v = r.value;
        document.documentElement.setAttribute("data-theme", v);
        localStorage.setItem("theme", v);
    }
}

const theme = localStorage.getItem("theme");
if (theme) {
    const radio = document.querySelector(`input[name="theme"][value="${theme}"]`);
    if (radio) radio.checked = true;
    document.documentElement.setAttribute("data-theme", theme);
}
