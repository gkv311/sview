/* Server API */

function invokeActionURL(endpoint, on_suceess, on_error) {
    fetch("/" + endpoint)
        .then(r => {
            if (!r.ok) throw new Error(`HTTP error! Status: ${r.status}`);
            return r.text();
        })
        .then(t => on_suceess?.(t))
        .catch(t => on_error?.(t));
}

/* Sending messages */

function emit(m) {
    window.postMessage(m, document.location.origin);
}

/* Receiving messages */

const messageIn = {
    "action:PlayPause": _ => invokeActionURL("play_pause"),

    "action:Stop": _ => invokeActionURL("stop"),

    "action:ListPrev": _ => invokeActionURL("prev"),

    "action:ListNext": _ => invokeActionURL("next"),

    "action:AudioMute": _ => invokeActionURL("mute"),

    "action:Fullscreen": _ => invokeActionURL("fullscr_win"),

    "op:PlaylistPlay": msg => invokeActionURL("item?" + msg.index),

    "op:Seek": msg => invokeActionURL("seek?" + msg.position.toFixed(2)),

    "op:SetVolume": msg => invokeActionURL("vol?" + Math.min(Math.max(0, msg.volume), 120)),
}

const actions_triggering_update = [
    "action:ListPrev",
    "action:ListNext",
    "op:PlaylistPlay",
    "op:SetVolume",
]

function listen_messages() {
    window.addEventListener("message", e => {
        if (e.origin == document.location.origin) {
            const a = e.data.type;
            messageIn[a]?.(e.data);
            if (actions_triggering_update.includes(a)) update();
        }
    });
}

/* Periodic update */

function auto_update() {
    invokeActionURL(
        "current?id",
        t => {
            const [playlist_id, file_id, volume] = t.split(':');
            last["online"].set(true);
            last["playlist_id"].set(Number(playlist_id));
            last["file_id"].set(Number(file_id));
            last["volume"].set(Number(volume));
        },
        _ => {
            last["online"].set(false);
            last["playlist_id"].set(-1);
            last["title"].set('');
        }
    );
}

let update_itv = -1;
function update() {
    if (update_itv > 0) clearInterval(update_itv);
    update_itv = window.setInterval(auto_update, 2000);
    window.setTimeout(auto_update, 100);
}

/* States */

let last_playlist = [];
const last = {
    "version": (v, s, _n) => s({ type: "info:Version", version: v }),

    "online": (v, s, _n) => s({ type: "info:Online", online: v }),

    "playlist_id": (v, s, n) => {
        if (v < 0) {
            if (last_playlist.length > 0) {
                s({
                    type: "info:Playlist",
                    titles: [],
                });
                last_playlist = [];
            }
            return;
        }
        if (n) {
            invokeActionURL("playlist", files => {
                const titles = files.split("\n").filter(str => str !== "");
                if (last_playlist.length != 0 || titles != 0) s({
                    type: "info:Playlist",
                    titles,
                });
                if (titles.length == 0) last['playlist_id'].set(-1);
                last_playlist = titles;
            });
        } else {
            s({
                type: "info:Playlist",
                titles: last_playlist,
            });
        }
    },

    "file_id": (v, s, n) => {
        s({
            type: "info:PlaylistIndex",
            index: v,
        });
        if (n) invokeActionURL("current?title", title => {
            last["title"].set(title);
        });
    },

    "title": (v, s, _n) => s({ type: "info:Title", title: v }),

    "volume": (v, s, _n) => s({ type: "info:Volume", volume: v }),
}

for (const k of Object.keys(last)) {
    const fct = last[k];
    last[k] = {
        value: undefined,
        set: v => {
            if (v !== last[k].value) {
                last[k].value = v;
                fct?.(v, emit, true);
            }
        },
        resend: _ => {
            fct?.(last[k].value, emit, false);
        }
    };
}

/* Init */

invokeActionURL("version", v => {
    if (v == "") return;
    last["version"].set(v);
    listen_messages();
    update();
})
