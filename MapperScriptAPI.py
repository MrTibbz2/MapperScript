#!/usr/bin/env python3
import requests
import json
import sys

class MapperScriptAPI:
    def __init__(self, url="http://localhost:18080/rpc"):
        self.url = url
        self.id = 1
    
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
    
    def upload_script(self, name, content):
        return self._call("uploadScript", [{"name": name, "content": content}])
    
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

def main():
    api = MapperScriptAPI()
    
    if len(sys.argv) < 2:
        print("Usage: python3 MapperScriptAPI.py <command> [args...]")
        print("Commands:")
        print("  upload <name> <content>")
        print("  scripts")
        print("  update <path> <content>")
        print("  run <path>")
        print("  plugins")
        print("  plugin <name>")
        print("  status")
        return
    
    cmd = sys.argv[1]
    
    if cmd == "upload" and len(sys.argv) == 4:
        result = api.upload_script(sys.argv[2], sys.argv[3])
    elif cmd == "scripts":
        result = api.get_scripts()
    elif cmd == "update" and len(sys.argv) == 4:
        result = api.update_script(sys.argv[2], sys.argv[3])
    elif cmd == "run" and len(sys.argv) == 3:
        result = api.run_script(sys.argv[2])
    elif cmd == "plugins":
        result = api.get_plugins_list()
    elif cmd == "plugin" and len(sys.argv) == 3:
        result = api.get_plugin(sys.argv[2])
    elif cmd == "status":
        result = api.get_status()
    else:
        print("Invalid command or arguments")
        return
    
    print(json.dumps(result, indent=2))

if __name__ == "__main__":
    main()