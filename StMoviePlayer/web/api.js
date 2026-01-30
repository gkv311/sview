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

const waiting_peers = {};

const peers = {
    [window.location.origin]: {
        obj: window,
        origin: window.location.origin,
    },
};

function emit_peer(peer, m) {
    peer?.obj?.postMessage(m, peer.origin);
}

function emit(m) {
    Object.values(peers).forEach(p => emit_peer(p, m));
}

/* Receiving messages */

const messageIn = {
    "acl:Accept": msg => {
        if (!(msg.origin in waiting_peers)) return;
        peers[msg.origin] = waiting_peers[msg.origin];
        delete waiting_peers[msg.origin];
        for (const k of ["version", "online", "volume", "playing", "muted", "playlist_id", "file_id", "title"])
            last[k].resend(peers[msg.origin]);
    },

    "acl:Reject": msg => {
        delete waiting_peers[msg.origin];
        emit_peer(peers[msg.origin], { type: "info:Online", online: false });
        delete peers[msg.origin];
    },

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
    "action:PlayPause",
    "action:Stop",
    "action:ListPrev",
    "action:ListNext",
    "action:AudioMute",
    "op:PlaylistPlay",
    "op:SetVolume",
]

function listen_messages() {
    window.addEventListener("message", e => {
        if (e.origin in peers) {
            const a = e.data.type;
            messageIn[a]?.(e.data);
            if (actions_triggering_update.includes(a)) update();

        } else if (e.data.type == "hello") {
            waiting_peers[e.origin] = { obj: e.source, origin: e.origin };
            emit({ ...e.data, type: "acl:Request", origin: e.origin });
        }
    });
}

/* Periodic update */

function auto_update() {
    invokeActionURL(
        "current?id",
        t => {
            const [playlist_id, file_id, volume, muted, playing] = t.split(':');
            last["online"].set(true);
            last["playlist_id"].set(Number(playlist_id));
            last["file_id"].set(Number(file_id));
            last["volume"].set(Number(volume));
            last["muted"].set(muted == 1);
            last["playing"].set(playing == 1);
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

    "muted": (v, s, _n) => s({ type: "info:Muted", muted: v }),

    "playing": (v, s, _n) => s({ type: "info:Playing", playing: v }),
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
        resend: peer => {
            fct?.(last[k].value, peer ? (m => emit_peer(peer, m)) : emit, false);
        }
    };
}

/* Init */

invokeActionURL("version", v => {
    if (v == "") return;
    last["version"].set(v);
    listen_messages();
    update();
    window.opener && window.opener.postMessage({ type: "ready", sview: true }, "*");
})
