import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import serial
import serial.tools.list_ports
import json
import zlib
import os
import threading
import time
import queue
import sys
from pathlib import Path

def append_log(text: str):
    print(text)

BAUD_RATE = 115200
APP_NAME = "CrowPanel"  

def settings_path() -> str:
    if sys.platform.startswith("win"):
        base = os.getenv("APPDATA") or os.path.expanduser("~")
    elif sys.platform == "darwin":
        base = os.path.join(os.path.expanduser("~"), "Library", "Application Support")
    else:
        base = os.path.join(os.path.expanduser("~"), ".config")

    folder = Path(base) / APP_NAME
    folder.mkdir(parents=True, exist_ok=True)
    return str(folder / "settings.json")

SETTINGS_FILE = settings_path()

stop_reader = False
ser = None
ser_lock = threading.Lock() 
last_rx_time = None

BG = "#646464"
DEFAULT_FONT = ("Arial", 13)
COMBO_BG = "#C0C0C0"
PROBE_MAX_MS = 200    
PROBE_POLL_MS = 20   

def safe_close_serial():
    global ser, stop_reader
    try:
        stop_reader = True
        if ser is not None:
            try:
                try:
                    ser.flush()
                except Exception:
                    pass
                ser.close()
            except Exception as e:
                append_log(f"Warning closing serial: {repr(e)}")
    finally:
        ser = None

def open_serial_if_needed(com_port: str):
    global ser, stop_reader
    with ser_lock:
        try:
            if ser and getattr(ser, "is_open", False):
                current = str(getattr(ser, "port", "")).lower()
                if current and current == str(com_port).lower():
                    append_log(f"open_serial_if_needed: already connected to {com_port}")
                    return
        except Exception:
            pass
        try:
            if ser and getattr(ser, "is_open", False):
                try:
                    current = str(getattr(ser, "port", "")).lower()
                except Exception:
                    current = ""
                if current and current != str(com_port).lower():
                    append_log(f"Switching serial from {current} to {com_port} — closing previous.")
                    try:
                        safe_close_serial()
                    except Exception as e:
                        append_log(f"Warning while closing previous serial: {repr(e)}")
        except Exception:
            pass
        attempts = 3
        s = None
        open_err = None
        for _ in range(attempts):
            s, open_err = try_open_serial_with_timeout(com_port, BAUD_RATE, open_timeout_s=0.25)
            if s is not None:
                break
            err_l = str(open_err or "").lower()
            if "permission" in err_l or "отказано" in err_l:
                time.sleep(0.15)
                continue
            else:
                break
        if s is None:
            append_log(f"Could not open {com_port}: {open_err}")
            raise serial.SerialException(f"Open failed: {open_err}")
        ser = s
        try:
            ser.timeout = 0.2
        except Exception:
            pass
        stop_reader = False
        start_read_thread()
        append_log(f"Connected to {com_port}")

def try_open_serial_with_timeout(com_port: str, baud: int, open_timeout_s: float = 0.25):
    q = queue.Queue()
    def _opener():
        try:
            s = serial.Serial(com_port, baud, timeout=0.2, write_timeout=0.05)
            q.put(("ok", s))
        except Exception as e:
            q.put(("err", repr(e)))

    th = threading.Thread(target=_opener, daemon=True)
    th.start()
    try:
        kind, payload = q.get(timeout=open_timeout_s)
    except queue.Empty:
        return None, f"Open timeout after {open_timeout_s:.2f}s"
    if kind == "ok":
        return payload, None
    else:
        return None, payload

def load_settings():
    if os.path.exists(SETTINGS_FILE):
        try:
            with open(SETTINGS_FILE, "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception:
            return {}
    return {}

def save_settings(data: dict):
    try:
        with open(SETTINGS_FILE, "w", encoding="utf-8") as f:
            json.dump(data, f, ensure_ascii=False, indent=2)
    except Exception as e:
        try:
            messagebox.showerror("Error", f"Couldn't save settings:{e}")
        except Exception:
            print("Couldn't save settings:", e)

def is_port_listed(com_port: str) -> bool:
    try:
        ports = get_com_ports()
        return any(p.lower() == str(com_port).lower() for p in ports)
    except Exception:
        return False

def can_open_temp_port(com_port: str, timeout_s: float = 0.25) -> bool:
    tmp, err = try_open_serial_with_timeout(com_port, BAUD_RATE, open_timeout_s=timeout_s)
    if tmp is None:
        append_log(f"can_open_temp_port: can't open {com_port}: {err}")
        return False
    try:
        try:
            tmp.close()
        except Exception:
            pass
        return True
    except Exception as e:
        append_log(f"can_open_temp_port: close error {repr(e)}")
        try:
            tmp.close()
        except Exception:
            pass
        return False

def get_port_info(com_port: str):
    try:
        for p in serial.tools.list_ports.comports():
            try:
                if p.device and p.device.lower() == str(com_port).lower():
                    return {
                        "device": p.device,
                        "hwid": getattr(p, "hwid", None),
                        "vid": ("%04X" % p.vid) if getattr(p, "vid", None) is not None else None,
                        "pid": ("%04X" % p.pid) if getattr(p, "pid", None) is not None else None,
                        "serial_number": getattr(p, "serial_number", None),
                        "description": getattr(p, "description", None),
                    }
            except Exception:
                continue
    except Exception as e:
        append_log(f"get_port_info error: {repr(e)}")
    return None

def device_signature(info: dict) -> str:
    if not info:
        return ""
    sn = info.get("serial_number")
    if sn:
        return f"SN:{sn}"
    vid = info.get("vid")
    pid = info.get("pid")
    hwid = info.get("hwid")
    if vid and pid:
        return f"VP:{vid}:{pid}"
    if hwid:
        return f"HW:{hwid}"
    desc = info.get("description")
    if desc:
        return f"DESC:{desc}"
    return ""

def save_last_port_if_possible(com_port: str):
    try:
        info = get_port_info(com_port)
        if not info:
            append_log(f"Not saving COM_PORT {com_port}: port not present.")
            return

        s = load_settings()
        s["COM_PORT"] = com_port
        s.pop("COM_PORT_SIG", None)
        save_settings(s)
        append_log(f"Saved COM_PORT {com_port} to settings (no signature).")
    except Exception as e:
        append_log(f"Warning saving COM_PORT: {repr(e)}")

def calculate_crc32_for_payload(obj: dict) -> str:
    pure = json.dumps(obj, separators=(',', ':'), ensure_ascii=False)
    crc = zlib.crc32(pure.encode('utf-8')) & 0xFFFFFFFF
    return format(crc, '08X')

def get_com_ports():
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]

def start_read_thread():
    def read_serial():
        global stop_reader, last_rx_time
        while not stop_reader:
            try:
                local_raw = None
                with ser_lock:
                    local_ser = ser
                    if local_ser and getattr(local_ser, "is_open", False) and getattr(local_ser, "in_waiting", 0):
                        try:
                            local_raw = local_ser.readline()
                        except Exception as e:
                            append_log(f"Serial read error while readline: {repr(e)}")
                if not local_raw:
                    time.sleep(0.05)
                    continue
                raw = local_raw
                try:
                    line = raw.decode('utf-8').strip()
                    if line:
                        append_log(line)
                except Exception:
                    hex_preview = raw.hex()
                    if len(hex_preview) > 200:
                        hex_preview = hex_preview[:200] + '...'
                    append_log(f"RX (hex): {hex_preview}")
                try:
                    last_rx_time = time.time()
                except Exception:
                    pass
            except serial.SerialException as e:
                append_log(f"Serial read error: {repr(e)}")
                try:
                    safe_close_serial()
                except Exception:
                    pass
                time.sleep(0.15)
                break
            except PermissionError as e:
                append_log(f"Serial permission error: {repr(e)}")
                try:
                    safe_close_serial()
                except Exception:
                    pass
                time.sleep(0.15)
                break
            except Exception as e:
                append_log(f"Serial read unexpected error: {repr(e)}")
                time.sleep(0.1)
    t = threading.Thread(target=read_serial, daemon=True)
    t.start()

def probe_port(com_port: str, probe_payload: str = None, timeout: float = None) -> (bool, str):
    if not probe_payload:
        probe_payload = "PING\n"

    total_wait = PROBE_MAX_MS / 1000.0
    poll_interval = PROBE_POLL_MS / 1000.0

    try:
        if ser and getattr(ser, "is_open", False):
            try:
                if str(getattr(ser, "port", "")).lower() == str(com_port).lower():
                    return True, None
            except Exception:
                pass

        tmp, open_err = try_open_serial_with_timeout(com_port, BAUD_RATE, open_timeout_s=0.25)
        if tmp is None:
            append_log(f"Probe error opening {com_port}: {open_err}")
            return False, str(open_err)

        try:
            tmp.timeout = poll_interval
            try:
                tmp.reset_input_buffer()
            except Exception:
                pass

            try:
                tmp.write(probe_payload.encode('utf-8'))
                try:
                    tmp.flush()
                except Exception:
                    pass
            except Exception as e:
                append_log(f"Probe write error on temp {com_port}: {repr(e)}")
                try:
                    tmp.close()
                except Exception:
                    pass
                return False, f"Probe write error: {repr(e)}"

            elapsed = 0.0
            while elapsed < total_wait:
                if getattr(tmp, "in_waiting", 0) > 0:
                    try:
                        resp = tmp.readline().decode('utf-8', errors='ignore').strip()
                    except Exception:
                        resp = ""
                    try:
                        tmp.close()
                    except Exception:
                        pass
                    if resp:
                        append_log(f"Probe response from {com_port}: {resp}")
                        return True, None
                    else:
                        append_log(f"Probe: data received on {com_port} but no newline")
                        return True, None
                time.sleep(poll_interval)
                elapsed += poll_interval

            try:
                tmp.close()
            except Exception:
                pass
            append_log(f"Probe: no response on {com_port}")
            return False, "No response"

        except Exception as e:
            try:
                tmp.close()
            except Exception:
                pass
            append_log(f"Probe unexpected error on {com_port}: {repr(e)}")
            return False, f"Unexpected probe error: {repr(e)}"

    except Exception as e:
        append_log(f"Probe unexpected outer error for {com_port}: {repr(e)}")
        return False, f"Unexpected error: {repr(e)}"

def send_json_payload(payload: dict, com_port: str, show_dialogs: bool = True) -> bool:
    global ser, stop_reader

    if not com_port:
        if show_dialogs:
            messagebox.showinfo(
                "No COM port selected",
                "No COM port is selected. Please open the Settings tab and choose a COM-port (use ⟳ to refresh the list)."
            )
        return False

    probe_ok, probe_err = probe_port(com_port, timeout=0.5)
    if not probe_ok:
        if probe_err:
            perr = str(probe_err)
            if "Open timeout" in perr or "timeout" in perr.lower():
                first_line = f"Port {com_port} is not responding."
            elif "PermissionError" in perr or "Отказано в доступе" in perr or "permission" in perr.lower():
                first_line = f"Access to {com_port} is denied — it may be used by another program."
            elif "Write timeout" in perr or "SerialTimeoutException" in perr or "write timeout" in perr.lower():
                first_line = f"Failed to write to {com_port} (write timeout)."
            else:
                first_line = f"Error communicating with {com_port}: {perr}"
        else:
            first_line = f"No response from {com_port}."

        if show_dialogs:
            messagebox.showinfo(
                "Device issue",
                f"{first_line}"
                "\n\nPlease open the Settings tab and choose a different COM port (press ⟳ to refresh the list), then try again."
            )
        else:
            append_log(f"Probe failed and show_dialogs=False — {probe_err}")
        return False

    crc = calculate_crc32_for_payload(payload)
    send_obj = dict(payload)
    send_obj["crc32"] = crc
    json_str = json.dumps(send_obj, separators=(',', ':'), ensure_ascii=False)

    try:
        open_fn = globals().get('open_serial_if_needed')
        if callable(open_fn):
            try:
                open_fn(com_port)
            except Exception as e:
                append_log(f"Write error: {repr(e)}")
                safe_close_serial()
                if show_dialogs:
                    messagebox.showerror("Error", f"Couldn't open the port {com_port}:{e}")
                append_log(f"Open port error: {e}")
                return False
        else:
            tmp, open_err = try_open_serial_with_timeout(com_port, BAUD_RATE, open_timeout_s=0.25)
            if tmp is None:
                append_log(f"Write error: couldn't open {com_port}: {open_err}")
                safe_close_serial()
                if show_dialogs:
                    messagebox.showerror("Error", f"Couldn't open the port {com_port}:{open_err}")
                return False
            with ser_lock:
                ser = tmp
                stop_reader = False
            start_read_thread()
            append_log(f"Connected to {com_port} (fallback opener)")

    except Exception as e:
        append_log(f"Write error: {repr(e)}")
        safe_close_serial()
        if show_dialogs:
            messagebox.showerror("Error", f"Couldn't open the port {com_port}:{e}")
        append_log(f"Open port error: {e}")
        return False

    try:
        try:
            with ser_lock:
                try:
                    ser.reset_input_buffer()
                except Exception:
                    pass

                append_log("TX: " + json_str)
                ser.write((json_str + '\n').encode('utf-8'))
                try:
                    ser.flush()
                except Exception:
                    pass
        except Exception as e:
            raise

        if show_dialogs:
            messagebox.showinfo("Complete", "WiFi settings sent to device.")

        save_last_port_if_possible(com_port)
        return True
    except serial.SerialException as e:
        append_log(f"Write error: {repr(e)}")
        if "Permission" in repr(e) or "Отказано" in repr(e) or "Access" in repr(e):
            try:
                safe_close_serial()
            except Exception:
                pass
        if show_dialogs:
            messagebox.showerror("Error", f"Couldn't write to the port {com_port}:{e}")
        return False

def on_send_wifi_settings(show_dialogs=True):
    ssid = ssid_entry.get().strip()
    password = password_entry.get().strip()
    
    if not ssid:
        if show_dialogs:
            messagebox.showerror("Error", "Please enter WiFi network name (SSID).")
        return
    
    com_port = combo_port.get().strip()
    if not com_port:
        if show_dialogs:
            messagebox.showinfo(
                "COM port not selected",
                "Please select a COM port in the Settings tab (open Settings and choose a port, press ⟳ to refresh)."
            )
        return
    
    payload = {
        "type": "wifi_config",
        "ssid": ssid,
        "password": password
    }
    
    send_json_payload(payload, com_port, show_dialogs)

def on_clear_wifi_settings():
    ssid_entry.delete(0, "end")
    password_entry.delete(0, "end")


def toggle_password_visibility():
    if password_entry.cget('show') == '*':
        password_entry.config(show='')
        show_hide_btn.config(text='Hide')
    else:
        password_entry.config(show='*')
        show_hide_btn.config(text='Show')

def refresh_ports():
    ports = get_com_ports()
    combo_port['values'] = ports
    if ports:
        if combo_port.get() not in ports:
            combo_port.set(ports[0])
    
    try:
        if ser and getattr(ser, "is_open", False):
            current = str(getattr(ser, "port", "")).lower()
            if current and current not in [p.lower() for p in ports]:
                append_log(f"Previously opened port {current} not in list after refresh — closing.")
                safe_close_serial()
    except Exception as e:
        append_log(f"Refresh ports: error checking current port: {repr(e)}")

root = tk.Tk()
root.title("WiFi Configuration Tool")

win_width = 550
win_height = 300 
root.geometry(f"{win_width}x{win_height}")

root.minsize(380, 280)
root.configure(bg=BG)

root.update_idletasks()  
screen_w = root.winfo_screenwidth()
screen_h = root.winfo_screenheight()
pos_x = max(0, (screen_w - win_width) // 2)
pos_y = max(0, (screen_h - win_height) // 2)
root.geometry(f"{win_width}x{win_height}+{pos_x}+{pos_y}")

style = ttk.Style()
style.theme_use('clam')

SELECTED_TAB_BG = BG            
UNSELECTED_TAB_BG = "#DCDCDC"   

style.configure('TNotebook.Tab', padding=(8,4), background=UNSELECTED_TAB_BG, foreground='black')
style.map('TNotebook.Tab',
          background=[('selected', SELECTED_TAB_BG), ('!selected', UNSELECTED_TAB_BG)],
          foreground=[('selected', 'white'), ('!selected', 'black')])

style.configure('LightGray.TCombobox',
                fieldbackground=COMBO_BG,
                background=COMBO_BG,
                foreground='black')
style.map('LightGray.TCombobox',
          fieldbackground=[('readonly', COMBO_BG), ('focus', COMBO_BG)],
          background=[('readonly', COMBO_BG), ('focus', COMBO_BG)],
          foreground=[('readonly', 'black'), ('focus', 'black')])

notebook = ttk.Notebook(root)
notebook.pack(fill="both", expand=True, padx=8, pady=8)

frame_wifi = tk.Frame(notebook, bg=BG)
notebook.add(frame_wifi, text="WiFi Configuration")

frame_wifi.columnconfigure(0, weight=1)
frame_wifi.rowconfigure(1, weight=0)
frame_wifi.rowconfigure(3, weight=0)
frame_wifi.rowconfigure(5, weight=1)

tk.Label(frame_wifi, text="WiFi Network (SSID):", font=DEFAULT_FONT, fg="white", bg=BG).grid(row=0, column=0, sticky="w", padx=8, pady=(8,4))
ssid_entry = tk.Entry(frame_wifi, font=DEFAULT_FONT)
ssid_entry.grid(row=1, column=0, sticky="ew", padx=8, pady=(0,8))

tk.Label(frame_wifi, text="Password:", font=DEFAULT_FONT, fg="white", bg=BG).grid(row=2, column=0, sticky="w", padx=8, pady=(8,4))

password_frame = tk.Frame(frame_wifi, bg=BG)
password_frame.grid(row=3, column=0, sticky="ew", padx=8, pady=(0,8))
password_frame.columnconfigure(0, weight=1) 

password_entry = tk.Entry(password_frame, font=DEFAULT_FONT, show="*")
password_entry.grid(row=0, column=0, sticky="ew", padx=(0, 8))

show_hide_btn = tk.Button(password_frame, text="Show", command=toggle_password_visibility, 
                         font=("Arial", 10), width=5)
show_hide_btn.grid(row=0, column=1, sticky="e")

btn_frame_wifi = tk.Frame(frame_wifi, bg=BG)
btn_frame_wifi.grid(row=4, column=0, sticky="w", padx=8, pady=(8,8))

send_button = tk.Button(btn_frame_wifi, text="Send WiFi Settings", command=lambda: on_send_wifi_settings(True), font=DEFAULT_FONT)
send_button.pack(side="left", padx=(0,8))
clear_button = tk.Button(btn_frame_wifi, text="Clear", command=on_clear_wifi_settings, font=DEFAULT_FONT)
clear_button.pack(side="left")

frame_settings = tk.Frame(notebook, bg=BG)
notebook.add(frame_settings, text="Settings")

settings_center = tk.Frame(frame_settings, bg=BG)
settings_center.place(relx=0.5, rely=0.45, anchor='center')

lbl_com = tk.Label(settings_center, text="COM-port:", font=DEFAULT_FONT, fg="white", bg=BG)
lbl_com.grid(row=0, column=0, sticky="e", padx=8, pady=8)

ports_list = get_com_ports()
combo_port = ttk.Combobox(settings_center, values=ports_list, state="readonly", width=28, font=DEFAULT_FONT, style='LightGray.TCombobox')
combo_port.grid(row=0, column=1, sticky="w", padx=(0,4), pady=8)

def on_port_selected(e):
    root.focus()

combo_port.bind('<<ComboboxSelected>>', on_port_selected)

refresh_btn = tk.Button(settings_center, text="⟳", command=refresh_ports, font=DEFAULT_FONT, width=3)
refresh_btn.grid(row=0, column=2, sticky="w", padx=(6,8), pady=8)

notebook.bind("<<NotebookTabChanged>>", lambda e: root.focus())

settings = load_settings()
saved_port = settings.get("COM_PORT")
ports_list = get_com_ports()
combo_port['values'] = ports_list
if saved_port and saved_port in ports_list:
    combo_port.set(saved_port)
elif ports_list:
    combo_port.current(0)

def on_close():
    safe_close_serial()
    root.destroy()

root.protocol("WM_DELETE_WINDOW", on_close)
root.mainloop()