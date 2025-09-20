# By John Hodge (thePowersGang), 2024
# Licensed under CC-BY-NC-SA 4.0

import sys
import os
import json
import traceback
import dearpygui.dearpygui as dpg
import MapperScriptAPI

mapperapi = MapperScriptAPI.MapperScriptAPI()

# ---------------------------------------
# Stdout Redirector
# ---------------------------------------
class StdoutRedirector:
    def __init__(self, log_parent):
        self.log_parent = log_parent

    def write(self, text):
        text = text.rstrip("\n")
        if text.strip() != "":
            for line in text.splitlines():
                if line.strip() != "":
                    dpg.add_text(line, parent=self.log_parent)
                    try:
                        dpg.set_y_scroll(self.log_parent, dpg.get_y_scroll_max(self.log_parent))
                    except Exception:
                        pass

    def flush(self): pass

# ---------------------------------------
# Globals
# ---------------------------------------
SCRIPTS_GROUP = "scripts_group"
PLUGINS_GROUP = "plugins_group"
LOG_WINDOW = "log_window"
FILE_DIALOG_ID = "file_dialog_id"
FOLDER_DIALOG_ID = "folder_dialog_id"

LAST_UPLOADED_SCRIPT = {"value": None}  # store last uploaded script
SCRIPT_RUNNING = {"value": False}       # ensures only one script runs at a time

# ---------------------------------------
# Helpers
# ---------------------------------------
def clear_log():
    if dpg.does_item_exist(LOG_WINDOW):
        dpg.delete_item(LOG_WINDOW, children_only=True)

# ---------------------------------------
# Script Handling
# ---------------------------------------
def refresh_scripts(sender=None, app_data=None, user_data=None):
    """Fetch scripts list from API and rebuild buttons"""
    try:
        result = mapperapi.get_scripts()
        scripts = []
        if isinstance(result, dict) and "result" in result:
            scripts = result["result"] or []
        elif isinstance(result, list):
            scripts = result
        else:
            scripts = result if result else []
    except Exception as e:
        print(f"[Error] Failed to fetch scripts: {e}")
        print(traceback.format_exc())
        scripts = []

    if dpg.does_item_exist(SCRIPTS_GROUP):
        dpg.delete_item(SCRIPTS_GROUP, children_only=True)

    for s in scripts:
        if isinstance(s, dict) and "path" in s:
            path = s["path"]
            label = os.path.basename(path)
        else:
            path = str(s)
            label = os.path.basename(path)

        btn_tag = f"script_btn::{path}"
        dpg.add_button(label=f"- {label}", parent=SCRIPTS_GROUP, tag=btn_tag,
                       callback=lambda s_, a, u, p=path: select_and_run_script(p))

    # Upload buttons
    dpg.add_button(label="Add Script (Upload)", parent=SCRIPTS_GROUP,
                   callback=lambda: dpg.show_item(FILE_DIALOG_ID))
    dpg.add_button(label="Add Script Folder (Upload)", parent=SCRIPTS_GROUP,
                   callback=lambda: dpg.show_item(FOLDER_DIALOG_ID))
    
    # Run last uploaded script button
    if LAST_UPLOADED_SCRIPT["value"]:
        dpg.add_button(label="Run Last Uploaded Script", parent=SCRIPTS_GROUP,
                       callback=lambda: run_script_by_path(LAST_UPLOADED_SCRIPT["value"]))

def select_and_run_script(path):
    print(f"Selected script: {path}")
    run_script_by_path(path)

def run_script_by_path(path):
    """Run a single script, only if none running, and print logs"""
    if SCRIPT_RUNNING["value"]:
        print("[Warning] A script is already running. Wait for it to finish.")
        return

    SCRIPT_RUNNING["value"] = True
    try:
        print(f"Running: {path}")
        resp = mapperapi.run_script(path)
        print(f"Result: {json.dumps(resp, indent=2)}")

        # Fetch and print logs
        try:
            logs = mapperapi.get_logs()
            if logs:
                print("\n--- Script Log ---")
                for line in logs:
                    print(line)
                print("--- End of Log ---\n")
        except Exception as log_err:
            print(f"[Error] Failed to fetch script logs: {log_err}")
            print(traceback.format_exc())

    except Exception as e:
        print(f"[Error] run_script failed: {e}")
        print(traceback.format_exc())
    finally:
        SCRIPT_RUNNING["value"] = False

def upload_file_callback(sender, app_data, user_data):
    """Upload a single script file"""
    file_path = app_data.get("file_path_name")
    file_name = app_data.get("file_name")
    if not file_path:
        print("[Upload] No file selected.")
        return

    try:
        with open(file_path, "r", encoding="utf-8") as fh:
            content = fh.read()
    except Exception as e:
        print(f"[Upload] Failed to read {file_path}: {e}")
        return

    api_path = f"scripts/{file_name}"
    try:
        res = mapperapi.upload_script(api_path, content)
        print(f"[Upload] Uploaded {file_name} as {api_path}: {json.dumps(res)}")
        LAST_UPLOADED_SCRIPT["value"] = api_path
        refresh_scripts()
    except Exception as e:
        print(f"[Upload] upload_script error: {e}")
        print(traceback.format_exc())

def upload_folder_callback(sender, app_data, user_data):
    """Upload all scripts from a selected folder"""
    folder_path = app_data.get("file_path_name")
    if not folder_path or not os.path.isdir(folder_path):
        print("[Upload] No folder selected.")
        return

    print(f"[Upload] Selected folder: {folder_path}")

    last_script = None
    for fname in os.listdir(folder_path):
        if not fname.lower().endswith((".lua", ".txt")):
            continue
        fpath = os.path.join(folder_path, fname)
        try:
            with open(fpath, "r", encoding="utf-8") as fh:
                content = fh.read()
            api_path = f"scripts/{fname}"
            res = mapperapi.upload_script(api_path, content)
            print(f"[Upload] Uploaded {fname} as {api_path}: {json.dumps(res)}")
            last_script = api_path
        except Exception as e:
            print(f"[Upload] Failed {fname}: {e}")
            print(traceback.format_exc())

    if last_script:
        LAST_UPLOADED_SCRIPT["value"] = last_script
    refresh_scripts()

# ---------------------------------------
# Plugin Handling
# ---------------------------------------
def refresh_plugins(sender=None, app_data=None, user_data=None):
    try:
        result = mapperapi.get_plugins_list()
        plugins = []
        if isinstance(result, dict) and "result" in result:
            plugins = result["result"] or []
        elif isinstance(result, list):
            plugins = result
        else:
            plugins = result if result else []
    except Exception as e:
        print(f"[Error] Failed to fetch plugins: {e}")
        print(traceback.format_exc())
        plugins = []

    if dpg.does_item_exist(PLUGINS_GROUP):
        dpg.delete_item(PLUGINS_GROUP, children_only=True)

    for p in plugins:
        if isinstance(p, dict) and "name" in p:
            name = p["name"]
        else:
            name = str(p)

        btn_tag = f"plugin_btn::{name}"
        dpg.add_button(label=f"- {name}", parent=PLUGINS_GROUP, tag=btn_tag,
                       callback=lambda s_, a, u, n=name: show_plugin_info(n))

def show_plugin_info(name):
    try:
        resp = mapperapi.get_plugin(name)
        description = json.dumps(resp, indent=2)
    except Exception as e:
        description = f"Error fetching plugin info: {e}"

    tag = f"{name}_info"
    if dpg.does_item_exist(tag):
        dpg.delete_item(tag)

    with dpg.window(label=f"{name} Info", tag=tag, width=400, height=300,
                    modal=True, no_collapse=True, no_resize=True):
        dpg.add_text(description, wrap=380)
        dpg.add_spacer(height=100)
        with dpg.group(horizontal=True):
            dpg.add_spacer(width=150)
            dpg.add_button(label="Close", width=75, callback=lambda: dpg.delete_item(tag))

mapperapi.start_logging()
print("MapperScript API Client - Logging started")

# ---------------------------------------
# GUI setup
# ---------------------------------------
dpg.create_context()
dpg.create_viewport(title="MapperScript Runner", width=1200, height=750)
dpg.configure_app(docking=True, docking_space=True)

with dpg.window(label="Scripts", tag="Scripts", width=400, height=450):
    dpg.add_text("Scripts:")
    with dpg.group(tag=SCRIPTS_GROUP):
        dpg.add_text("No scripts fetched")
    dpg.add_separator()
    dpg.add_button(label="Refresh Scripts", callback=refresh_scripts)

with dpg.window(label="Plugins", tag="Plugins", width=400, height=450, pos=(420, 0)):
    dpg.add_text("Plugins:")
    with dpg.group(tag=PLUGINS_GROUP):
        dpg.add_text("No plugins fetched")
    dpg.add_separator()
    dpg.add_button(label="Refresh Plugins", callback=refresh_plugins)

with dpg.window(label="Log", tag="ScriptRunner", width=700, height=600, pos=(0, 460)):
    dpg.add_text("Output Log:", bullet=True)
    with dpg.child_window(tag=LOG_WINDOW, width=-1, height=500, autosize_x=True):
        pass
    dpg.add_separator()
    dpg.add_button(label="Clear Log", callback=clear_log)

# File dialog (single file)
with dpg.file_dialog(directory_selector=False, show=False, callback=upload_file_callback,
                     id=FILE_DIALOG_ID, width=700, height=400):
    dpg.add_file_extension(".lua")
    dpg.add_file_extension(".txt")

# Folder dialog
with dpg.file_dialog(directory_selector=True, show=False, callback=upload_folder_callback,
                     id=FOLDER_DIALOG_ID, width=700, height=400):
    dpg.add_file_extension("")

# Redirect print
sys.stdout = StdoutRedirector(LOG_WINDOW)
sys.stderr = StdoutRedirector(LOG_WINDOW)

# Start GUI
dpg.setup_dearpygui()
dpg.show_viewport()
refresh_scripts()
refresh_plugins()
dpg.start_dearpygui()
dpg.destroy_context()
