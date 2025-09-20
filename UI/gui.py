#!/usr/bin/env python3
"""
MapperScriptUI.py

Single-file PyWebView UI for MapperScript API (uses MapperScriptAPI from MapperScriptAPI.py).
Dark, minimal, single-page UI with Plugins, Scripts, and Logs.

Run: python3 MapperScriptUI.py

Requirements: pywebview, requests, websocket-client (MapperScriptAPI auto-installs requests & websocket-client)
Install pywebview: pip install pywebview

"""

import threading
import time
import json
import sys
import os
import traceback
from pathlib import Path

try:
    import webview
except Exception:
    print("pywebview is required. Install with: pip install pywebview")
    raise

# Import the provided MapperScriptAPI - expects MapperScriptAPI.py in the same directory
from MapperScriptAPI import MapperScriptAPI

class MSApiWrapper:
    """Python API exposed to JS through pywebview. Wraps MapperScriptAPI with safe error handling.
    Methods should return JSON-serializable objects.
    """
    def __init__(self, url=None):
        self.ms = MapperScriptAPI(url or "http://localhost:18080/rpc")
        # start websocket logging - this will populate self.ms.logs
        try:
            self.ms.start_logging()
        except Exception as e:
            print("Failed to start MS logging:", e)
        # small lock to protect ms calls that aren't thread-safe
        import threading
        self._lock = threading.Lock()

    def _safe(self, fn, *args, **kwargs):
        try:
            with self._lock:
                res = fn(*args, **kwargs)
            return {"ok": True, "result": res}
        except Exception as e:
            tb = traceback.format_exc()
            print(tb)
            return {"ok": False, "error": str(e), "traceback": tb}

    # Plugins
    def get_plugins(self):
        return self._safe(self.ms.get_plugins_list)

    def get_plugin(self, name):
        return self._safe(self.ms.get_plugin, name)

    def refresh_plugins(self):
        # same as get_plugins
        return self.get_plugins()

    # Scripts
    def get_scripts(self):
        return self._safe(self.ms.get_scripts)

    def run_script(self, path):
        # runScript may fail if script already running or other error; return full response
        try:
            with self._lock:
                res = self.ms.run_script(path)
            # mapper returns jsonrpc style dict; interpret
            if isinstance(res, dict) and res.get("error"):
                return {"ok": False, "error": res.get("error")}
            # success indicated by result.success usually
            if isinstance(res, dict) and res.get("result") and isinstance(res["result"], dict):
                if not res["result"].get("success", True):
                    return {"ok": False, "error": res["result"].get("message", "failed to start") , "raw": res}
            return {"ok": True, "result": res}
        except Exception as e:
            tb = traceback.format_exc()
            return {"ok": False, "error": str(e), "traceback": tb}

    def upload_script(self, path, content):
        # path should start with /, ensure it's safe
        if not path.startswith("/"):
            path = "/" + path
        return self._safe(self.ms.upload_script, path, content)

    def update_script(self, path, content):
        if not path.startswith("/"):
            path = "/" + path
        return self._safe(self.ms.update_script, path, content)

    # Logs
    def get_logs(self):
        try:
            with self._lock:
                logs = self.ms.get_logs()
            # ensure timestamps are integers and messages are strings
            out = []
            for l in logs:
                if isinstance(l, dict):
                    out.append({
                        "message": str(l.get("message", l.get("msg", ""))),
                        "timestamp": int(l.get("timestamp", 0))
                    })
            return {"ok": True, "result": out}
        except Exception as e:
            tb = traceback.format_exc()
            return {"ok": False, "error": str(e), "traceback": tb}

    def clear_logs(self):
        try:
            with self._lock:
                # deque has clear in Python 3.6+
                if hasattr(self.ms.logs, 'clear'):
                    self.ms.logs.clear()
                else:
                    self.ms.logs = self.ms.logs.__class__(maxlen=1000)
            return {"ok": True}
        except Exception as e:
            tb = traceback.format_exc()
            return {"ok": False, "error": str(e), "traceback": tb}

    def get_status(self):
        return self._safe(self.ms.get_status)

# HTML UI: single-file, minimal and dark
HTML = r"""
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width,initial-scale=1" />
<title>MapperScript UI</title>
<style>
:root{--bg:#0b0d0f;--panel:#0f1215;--muted:#9aa3ad;--accent:#6ee7b7;--danger:#ff6b6b;--glass:rgba(255,255,255,0.03)}
html,body{height:100%;margin:0;background:linear-gradient(180deg,#060708 0%,#0b0d0f 100%);font-family:Inter,ui-sans-serif,system-ui,-apple-system,'Segoe UI',Roboto,'Helvetica Neue',Arial;}
.container{display:grid;grid-template-columns:240px 1fr;gap:18px;height:100vh;padding:18px;box-sizing:border-box}
.sidebar{background:var(--panel);border-radius:12px;padding:12px;display:flex;flex-direction:column;gap:8px;box-shadow:0 6px 18px rgba(2,6,23,0.6)}
.brand{font-weight:700;color:var(--accent);font-size:16px;padding:6px 8px}
.nav{display:flex;flex-direction:column;gap:6px;margin-top:6px}
.btn{background:transparent;border:0;padding:9px 10px;border-radius:8px;color:var(--muted);cursor:pointer;text-align:left}
.btn:hover{background:var(--glass);color:#ffffff}
.btn.active{background:linear-gradient(90deg,rgba(110,231,183,0.08),transparent);color:#fff}
.main{display:flex;flex-direction:column;gap:12px}
.header{display:flex;align-items:center;justify-content:space-between;color:var(--muted)}
.card{background:var(--panel);padding:12px;border-radius:12px;box-shadow:0 6px 18px rgba(2,6,23,0.45);color:#dfe7ea}
.row{display:flex;gap:8px;align-items:center}
.title{font-weight:600;color:#fff}
.small{font-size:13px;color:var(--muted)}
.split{display:grid;grid-template-columns:1fr 1fr;gap:12px}
.list{max-height:56vh;overflow:auto;padding:6px;border-radius:8px;background:linear-gradient(180deg,rgba(255,255,255,0.01),transparent)}
.item{padding:10px;border-radius:8px;display:flex;justify-content:space-between;align-items:center;margin-bottom:6px}
.item:hover{background:rgba(255,255,255,0.01);cursor:pointer}
.tag{font-size:12px;padding:6px 8px;border-radius:999px;background:rgba(255,255,255,0.03);color:var(--muted)}
.actions{display:flex;gap:8px}
.inputfile{display:none}
.modal{position:fixed;inset:0;background:rgba(1,2,3,0.6);display:flex;align-items:center;justify-content:center}
.modal-card{width:680px;max-width:95%;background:var(--panel);padding:18px;border-radius:12px}
.toast{position:fixed;right:20px;bottom:20px;background:#111;padding:12px;border-radius:10px;border:1px solid rgba(255,255,255,0.03);color:#fff}
.empty{color:var(--muted);padding:18px;text-align:center}
.log-line{font-family:monospace;font-size:13px;padding:6px 0;border-bottom:1px dashed rgba(255,255,255,0.02)}
.footer{font-size:12px;color:var(--muted);padding:6px}
</style>
</head>
<body>
<div class="container">
  <div class="sidebar card">
    <div class="brand">MapperScript — Minimal UI</div>
    <div class="nav">
      <button id="tab-plugins" class="btn active">Plugins</button>
      <button id="tab-scripts" class="btn">Scripts</button>
      <button id="tab-logs" class="btn">Logs</button>
    </div>
    <div style="flex:1"></div>
    <div class="small">Status: <span id="status-text">connecting...</span></div>
    <div class="footer">Built for MapperScript — dark, minimal, single page</div>
  </div>

  <div class="main">
    <div class="header">
      <div>
        <div class="title" id="page-title">Plugins</div>
        <div class="small" id="page-sub">View and inspect plugins</div>
      </div>
      <div class="row">
        <div id="controls"></div>
      </div>
    </div>

    <div id="content-area">
      <!-- Plugins view -->
      <div id="view-plugins" class="card view">
        <div class="row" style="justify-content:space-between;align-items:center;margin-bottom:8px">
          <div class="small">Loaded plugins</div>
          <div class="actions">
            <button id="refresh-plugins" class="btn">Refresh</button>
          </div>
        </div>
        <div id="plugins-list" class="list"></div>
        <div id="plugins-empty" class="empty" style="display:none">No plugins found</div>
      </div>

      <!-- Scripts view -->
      <div id="view-scripts" class="card view" style="display:none">
        <div class="row" style="justify-content:space-between;align-items:center;margin-bottom:8px">
          <div class="small">Manage scripts</div>
          <div class="actions">
            <label class="btn" for="upload-file">Upload Lua</label>
            <input id="upload-file" class="inputfile" type="file" accept=".lua" />
            <button id="refresh-scripts" class="btn">Refresh</button>
          </div>
        </div>
        <div id="scripts-list" class="list"></div>
        <div id="scripts-empty" class="empty" style="display:none">No scripts found</div>
      </div>

      <!-- Logs view -->
      <div id="view-logs" class="card view" style="display:none">
        <div class="row" style="justify-content:space-between;align-items:center;margin-bottom:8px">
          <div class="small">Realtime logs</div>
          <div class="actions">
            <button id="clear-logs" class="btn">Clear</button>
          </div>
        </div>
        <div id="logs-list" class="list" style="font-size:13px"></div>
        <div id="logs-empty" class="empty" style="display:none">No logs yet</div>
      </div>
    </div>
  </div>
</div>

<!-- Plugin modal -->
<div id="modal" class="modal" style="display:none">
  <div class="modal-card">
    <div class="row" style="justify-content:space-between;margin-bottom:12px">
      <div class="title">Plugin</div>
      <div class="small"><button id="close-modal" class="btn">Close</button></div>
    </div>
    <pre id="modal-body" style="white-space:pre-wrap;font-size:13px;color:var(--muted)"></pre>
  </div>
</div>

<div id="toast" class="toast" style="display:none"></div>

<script>
// Helper UI functions
const el = id => document.getElementById(id);
function showToast(text, timeout=4000){
  const t = el('toast');
  t.textContent = text;
  t.style.display = 'block';
  clearTimeout(t._timer);
  t._timer = setTimeout(()=> t.style.display='none', timeout);
}

// Tab logic
const tabs = ['plugins','scripts','logs'];
for(const t of tabs){
  el('tab-'+t).addEventListener('click', ()=> switchTab(t));
}
function switchTab(tab){
  for(const t of tabs){
    el('tab-'+t).classList.toggle('active', t===tab);
    el('view-'+t).style.display = (t===tab)?'block':'none';
  }
  const titles = {plugins:['Plugins','View and inspect plugins'], scripts:['Scripts','Manage scripts'], logs:['Logs','Realtime logs']};
  el('page-title').textContent = titles[tab][0];
  el('page-sub').textContent = titles[tab][1];
}

// API convenience
async function call(name, ...args){
  try{
    const fn = window.pywebview.api[name];
    if(!fn) throw new Error('API function not available: '+name);
    const res = await fn(...args);
    return res;
  }catch(e){
    console.error(e);
    return {ok:false,error: e.toString()};
  }
}

// Plugins
async function loadPlugins(){
  el('plugins-list').innerHTML='';
  el('plugins-empty').style.display='none';
  const r = await call('get_plugins');
  if(!r.ok){ showToast('Failed to load plugins: '+(r.error||r.result)); return; }
  const data = r.result.result || r.result; // wrapper
  if(!data || data.length===0){ el('plugins-empty').style.display='block'; return; }
  for(const p of data){
    const it = document.createElement('div'); it.className='item';
    const left = document.createElement('div');
    left.innerHTML = `<div style="font-weight:600">${escapeHtml(p.name||p['name']||'Unnamed')}</div><div class='small'>${p.loaded? 'Loaded' : 'Not loaded'}</div>`;
    const right = document.createElement('div');
    right.innerHTML = `<span class='tag'>${p.loaded? 'loaded' : 'unloaded'}</span>`;
    it.appendChild(left); it.appendChild(right);
    it.addEventListener('click', ()=> showPlugin(p.name||p['name']));
    el('plugins-list').appendChild(it);
  }
}

async function showPlugin(name){
  const r = await call('get_plugin', name);
  if(!r.ok){ showToast('Failed to load plugin: '+(r.error||r.result)); return; }
  const p = r.result.result || r.result;
  el('modal-body').textContent = JSON.stringify(p, null, 2);
  el('modal').style.display='flex';
}

el('close-modal').addEventListener('click', ()=> el('modal').style.display='none');

el('refresh-plugins').addEventListener('click', ()=> loadPlugins());

// Scripts
async function loadScripts(){
  el('scripts-list').innerHTML='';
  el('scripts-empty').style.display='none';
  const r = await call('get_scripts');
  if(!r.ok){ showToast('Failed to load scripts: '+(r.error||r.result)); return; }
  const data = r.result.result || r.result;
  if(!data || data.length===0){ el('scripts-empty').style.display='block'; return; }
  for(const s of data){
    const it = document.createElement('div'); it.className='item';
    const left = document.createElement('div');
    const name = s.name||s.path||s['name']||'unnamed';
    left.innerHTML = `<div style="font-weight:600">${escapeHtml(name)}</div><div class='small'>${escapeHtml((s.content||'').slice(0,120))}</div>`;
    const right = document.createElement('div');
    right.innerHTML = `<div class='actions'><button class='btn' onclick="runScript(event,'${escapeJs(name)}')">Run</button></div>`;
    it.appendChild(left); it.appendChild(right);
    el('scripts-list').appendChild(it);
  }
}

async function runScript(event, path){
  event.stopPropagation();
  showToast('Starting script...');
  const r = await call('run_script', path);
  if(!r.ok){ showToast('Failed to start: '+(r.error||r.result)); return; }
  // r.result may contain jsonrpc style result
  if(r.result && r.result.result && r.result.result.success===false){
    showToast('Script failed to start: '+(r.result.result.message||'unknown'));
    return;
  }
  showToast('Script started');
}

el('refresh-scripts').addEventListener('click', ()=> loadScripts());

// Upload file
el('upload-file').addEventListener('change', async function(e){
  const f = this.files[0];
  if(!f) return;
  const txt = await f.text();
  const path = '/' + f.name;
  showToast('Uploading '+f.name);
  const r = await call('upload_script', path, txt);
  if(!r.ok){ showToast('Upload failed: '+(r.error||r.result)); return; }
  showToast('Upload OK');
  loadScripts();
});

// Logs
let logsPoll = null;
async function loadLogs(){
  const r = await call('get_logs');
  if(!r.ok){ console.error('logs err', r); return; }
  const data = r.result.result || r.result;
  const list = el('logs-list'); list.innerHTML='';
  if(!data || data.length===0){ el('logs-empty').style.display='block'; return; } else el('logs-empty').style.display='none';
  // show newest last
  for(const l of data){
    const d = document.createElement('div'); d.className='log-line';
    const ts = new Date((l.timestamp||0)*1000);
    d.textContent = `[${ts.toLocaleTimeString()}] ${l.message}`;
    list.appendChild(d);
  }
  // auto-scroll to bottom
  list.scrollTop = list.scrollHeight;
}

el('clear-logs').addEventListener('click', async ()=>{
  const r = await call('clear_logs');
  if(!r.ok){ showToast('Clear failed'); return; }
  showToast('Logs cleared');
  loadLogs();
});

// Utilities
function escapeHtml(s){ return (s+'').replace(/[&<>"']/g, function(c){ return {'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":"&#39;"}[c]; }); }
function escapeJs(s){ return (s+'').replace(/\\/g,'\\\\').replace(/'/g,"\\'"); }

// Initialisation
async function init(){
  // show status
  try{
    const st = await call('get_status');
    if(st.ok && st.result && st.result.result){ el('status-text').textContent = st.result.result.status || 'ok'; }
    else if(st.ok && st.result.status){ el('status-text').textContent = st.result.status; }
    else el('status-text').textContent = 'connected';
  }catch(e){ el('status-text').textContent = 'offline'; }

  await loadPlugins();
  await loadScripts();
  await loadLogs();

  // Poll logs frequently
  if(logsPoll) clearInterval(logsPoll);
  logsPoll = setInterval(loadLogs, 1200);
}

// Start
window.addEventListener('pywebviewready', init);
</script>
</body>
</html>
"""

def start_ui(ms_url=None):
    api = MSApiWrapper(ms_url)
    # create webview window
    window = webview.create_window('MapperScript UI', html=HTML, js_api=api, width=1100, height=700, min_size=(800,500))
    webview.start(gui='qt')

if __name__ == '__main__':
    start_ui()
