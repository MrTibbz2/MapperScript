#!/usr/bin/env python3
import subprocess
import sys

# Auto-install required packages
required_packages = [('requests', 'requests'), ('websocket-client', 'websocket')]
for package, module in required_packages:
    try:
        __import__(module)
    except ImportError:
        print(f"Installing {package}...")
        subprocess.check_call([sys.executable, '-m', 'pip', 'install', '--break-system-packages', package])

import requests
import json
import time
import websocket
import threading
from collections import deque

class MapperScriptAPI:
    def __init__(self, url="http://localhost:18080/rpc"):
        self.url = url
        self.id = 1
        self.logs = deque(maxlen=1000)
        self.ws = None
        self.ws_thread = None
        self.reconnect_thread = None
        self.should_reconnect = False
        self.connected = False
    
    def _call(self, method, params=None):
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params or [],
            "id": self.id
        }
        self.id += 1
        
        try:
            response = requests.post(self.url, json=payload)
            return response.json()
        except Exception as e:
            return {"error": str(e)}
    
    def upload_script(self, path, content):
        return self._call("uploadScript", [{"path": path, "content": content}])
    
    def get_scripts(self):
        return self._call("getScripts")
    
    def update_script(self, path, content):
        return self._call("updateScript", [{"path": path, "content": content}])
    
    def run_script(self, path):
        return self._call("runScript", [path])
    
    def get_plugins_list(self):
        return self._call("getPluginsList")
    
    def get_plugin(self, name):
        return self._call("getPlugin", [name])
    
    def get_status(self):
        return self._call("getStatus")
    
    def start_logging(self):
        self.should_reconnect = True
        self._connect_ws()
        
        def reconnect_loop():
            while self.should_reconnect:
                time.sleep(3)
                if not self.connected and self.should_reconnect:
                    self._connect_ws()
        
        self.reconnect_thread = threading.Thread(target=reconnect_loop)
        self.reconnect_thread.daemon = True
        self.reconnect_thread.start()
    
    def _connect_ws(self):
        ws_url = self.url.replace("http://", "ws://").replace("/rpc", "/logs")
        
        def on_message(ws, message):
            try:
                log_data = json.loads(message)
                self.logs.append(log_data)
            except:
                pass
        
        def on_open(ws):
            self.connected = True
        
        def on_close(ws, close_status_code, close_msg):
            self.connected = False
        
        def on_error(ws, error):
            self.connected = False
        
        try:
            self.ws = websocket.WebSocketApp(ws_url, on_message=on_message, on_open=on_open, on_close=on_close, on_error=on_error)
            self.ws_thread = threading.Thread(target=self.ws.run_forever)
            self.ws_thread.daemon = True
            self.ws_thread.start()
        except:
            self.connected = False
    
    def get_logs(self):
        return list(self.logs)
    
    def stop_logging(self):
        self.should_reconnect = False
        self.connected = False
        if self.ws:
            self.ws.close()

def main():
    api = MapperScriptAPI()
    api.start_logging()
    print("MapperScript API Client - Logging started")
    
    while True:
        print("\n=== MapperScript API ===")
        print("1. Get Scripts")
        print("2. Upload Script")
        print("3. Update Script")
        print("4. Run Script")
        print("5. Get Plugins")
        print("6. Get Plugin Info")
        print("7. Get Status")
        print("8. View Logs")
        print("9. Exit")
        
        try:
            choice = input("\nSelect option: ").strip()
        except (EOFError, KeyboardInterrupt):
            api.stop_logging()
            break
        
        if choice == "1":
            result = api.get_scripts()
        elif choice == "2":
            path = input("Script path: ")
            content = input("Script content: ")
            result = api.upload_script(path, content)
        elif choice == "3":
            path = input("Script path: ")
            content = input("New content: ")
            result = api.update_script(path, content)
        elif choice == "4":
            path = input("Script path: ")
            result = api.run_script(path)
        elif choice == "5":
            result = api.get_plugins_list()
        elif choice == "6":
            name = input("Plugin name: ")
            result = api.get_plugin(name)
        elif choice == "7":
            result = api.get_status()
        elif choice == "8":
            result = api.get_logs()
        elif choice == "9":
            api.stop_logging()
            break
        else:
            print("Invalid option")
            continue
        
        print("\n--- Result ---")
        print(json.dumps(result, indent=2))
        try:
            input("\nPress Enter to continue...")
        except (EOFError, KeyboardInterrupt):
            api.stop_logging()
            break

if __name__ == "__main__":
    main()