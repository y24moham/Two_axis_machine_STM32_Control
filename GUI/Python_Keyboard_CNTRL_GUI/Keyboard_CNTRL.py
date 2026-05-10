import time
import threading
import serial
from serial.tools import list_ports
import keyboard
import tkinter as tk
from tkinter import ttk

# Constants and configuration

# Main settings
DEFAULT_SERIAL_PORT = "COM5"
DEFAULT_SERIAL_BAUDRATE = "115200"
POLL_TIME_S = 0.01
RESEND_STOP_INTERVAL_S = 0.5 # how often to resend the stop command while in stop state to ensure the STM32 stays stopped if it misses a command or encounters an error (set to 0.5 seconds for more responsive stopping while still preventing command flooding)

# Keys for X movement
KEY_X_NEG = 'a'
KEY_X_POS = 'd'

# Keys for Y movement
KEY_Y_POS = 'w'
KEY_Y_NEG = 's'

# Commands sent to the STM32
CMD_X_NEG  = 'XN'
CMD_X_POS  = 'XP'
CMD_X_STOP = 'XS'

CMD_Y_POS  = 'YP'
CMD_Y_NEG  = 'YN'
CMD_Y_STOP = 'YS'

CMD_NORMAL_CONTROL = 'NRMLCNTRL'

BAUD_OPTIONS = [
    "9600",
    "19200",
    "38400",
    "57600",
    "115200",
    "230400",
    "460800",
    "921600"
]

# Shared state and synchronization
ser = None
program_alive = True
start_mode = False

last_x_state = None
last_y_state = None
last_x_stop_sent_time = 0.0
last_y_stop_sent_time = 0.0

# Lock for synchronizing access to shared GUI data between threads
lock = threading.Lock()

# GUI state data dictionary to hold the current state of keys, motion, last sent command, and error messages
gui_data = {
    "w_pressed": False,
    "a_pressed": False,
    "s_pressed": False,
    "d_pressed": False,
    "x_state": "STOP",
    "y_state": "STOP",
    "arrow": "•",
    "motion_text": "Idle",
    "last_sent": "None",
    "error_text": ""
}

# Helper functions

def set_error(msg: str):
    # sets an error message in the GUI, thread-safe.
    with lock:
        gui_data["error_text"] = msg

def clear_error():
    # clears the error message in the GUI, thread-safe.
    with lock:
        gui_data["error_text"] = ""

def list_available_ports():
    # Returns a sorted list of available serial ports. If no ports are found, returns a list with an empty string.
    ports = sorted([p.device for p in list_ports.comports()])
    return ports if ports else [""]

def refresh_ports():
    # Refreshes the list of available serial ports in the dropdown and tries to keep the current selection if possible.
    ports = list_available_ports()
    current = port_var.get()

    port_combo["values"] = ports

    if current in ports and current != "":
        port_var.set(current)
    else:
        if DEFAULT_SERIAL_PORT in ports:
            port_var.set(DEFAULT_SERIAL_PORT)
        else:
            port_var.set(ports[0] if ports else "")

def safe_close_serial():
    # Safely closes the serial port if it's open, and sets the global ser variable to None.
    global ser
    try:
        if ser is not None and ser.is_open:
            ser.close()
    except Exception:
        pass
    ser = None

def send_cmd(cmd: str):
    # Sends a command string to the STM32 over serial, with error handling. Also updates the last sent command in the GUI.
    global ser

    if ser is None or not ser.is_open:
        raise serial.SerialException("Serial port is not open.")

    if cmd is None:
        return
    ser.write((cmd + '\r').encode('utf-8'))
    print(f"Sent: {cmd}")

    with lock:
        gui_data["last_sent"] = cmd

def get_x_state():
    # Determines the current X movement state based on the A and D keys. Returns "NEG", "POS", or "STOP".
    x_neg_pressed = keyboard.is_pressed(KEY_X_NEG)
    x_pos_pressed = keyboard.is_pressed(KEY_X_POS)

    if x_neg_pressed and x_pos_pressed:
        return "STOP"
    elif x_neg_pressed:
        return "NEG"
    elif x_pos_pressed:
        return "POS"
    else:
        return "STOP"

def get_y_state():
    # Determines the current Y movement state based on the W and S keys. Returns "NEG", "POS", or "STOP".
    y_neg_pressed = keyboard.is_pressed(KEY_Y_NEG)
    y_pos_pressed = keyboard.is_pressed(KEY_Y_POS)

    if y_neg_pressed and y_pos_pressed:
        return "STOP"
    elif y_neg_pressed:
        return "NEG"
    elif y_pos_pressed:
        return "POS"
    else:
        return "STOP"

def get_motion_visual(x_state, y_state):
    # Returns a tuple of (arrow_symbol, motion_text) based on the current X and Y states for visual feedback in the GUI, with if statements covering all cases.
    if x_state == "STOP" and y_state == "STOP":
        return "•", "Stopped"
    if x_state == "STOP" and y_state == "POS":
        return "↑", "Up"
    if x_state == "STOP" and y_state == "NEG":
        return "↓", "Down"
    if x_state == "POS" and y_state == "STOP":
        return "→", "Right"
    if x_state == "NEG" and y_state == "STOP":
        return "←", "Left"
    if x_state == "POS" and y_state == "POS":
        return "↗", "Up-Right"
    if x_state == "NEG" and y_state == "POS":
        return "↖", "Up-Left"
    if x_state == "POS" and y_state == "NEG":
        return "↘", "Down-Right"
    if x_state == "NEG" and y_state == "NEG":
        return "↙", "Down-Left"
    return "•", "Stopped"

def update_button_states():
    # Updates the enabled/disabled state and colors of the Start/Stop buttons and related controls based on whether start_mode is active.
    if start_mode:
        start_button.config(state="disabled", bg="#555555", fg="#cccccc")
        stop_button.config(state="normal", bg="#d9534f", fg="white")
        port_combo.config(state="disabled")
        baud_combo.config(state="disabled")
        refresh_ports_button.config(state="disabled")
        mode_label.config(text="Mode: ACTIVE", fg="#33c481")
    else:
        start_button.config(state="normal", bg="#33c481", fg="black")
        stop_button.config(state="disabled", bg="#555555", fg="#cccccc")
        port_combo.config(state="readonly")
        baud_combo.config(state="readonly")
        refresh_ports_button.config(state="normal")
        mode_label.config(text="Mode: IDLE", fg="#aaaaaa")

def enter_idle_visual_state():
    # Resets the GUI visual state to idle (no keys pressed, stopped motion, control mode deactivated; GUI in stopped state) in a thread-safe way.
    with lock:
        gui_data["w_pressed"] = False
        gui_data["a_pressed"] = False
        gui_data["s_pressed"] = False
        gui_data["d_pressed"] = False
        gui_data["x_state"] = "STOP"
        gui_data["y_state"] = "STOP"
        gui_data["arrow"] = "•"
        gui_data["motion_text"] = "Idle"

def internal_stop(send_normal_twice=True, error_message=""):
    # Internal function to handle stopping the control mode, sending the normal control command 3 times. Resets state and updates GUI accordingly.
    global start_mode, last_x_state, last_y_state

    start_mode = False
    last_x_state = None
    last_y_state = None

    if error_message:
        set_error(error_message)
    else:
        clear_error()

    # On a normal stop, send NRMLCNTRL twice before dropping out
    if send_normal_twice:
        try:
            if ser is not None and ser.is_open:
                send_cmd(CMD_NORMAL_CONTROL)
                time.sleep(0.05)
                send_cmd(CMD_NORMAL_CONTROL)
                root.after(5000, lambda: send_cmd(CMD_NORMAL_CONTROL) if (start_mode == False) else None)  # one last backup send after 5 seconds
        except Exception:
            pass

    # safe_close_serial()
    refresh_ports()
    enter_idle_visual_state()
    update_button_states()

def handle_serial_failure(msg):
    # Handles serial communication failures by stopping the control mode and displaying an error message (does not send the normal control command).
    internal_stop(send_normal_twice=False, error_message=msg)


# Start / stop / refresh button action handlers

def on_start():
    # Handles the Start button click: validates settings, opens serial port (if not already open), sends initial stop commands, and sets up state for active control mode. On failure, shows error and resets to idle.
    global ser, start_mode
    global last_x_state, last_y_state
    global last_x_stop_sent_time, last_y_stop_sent_time

    clear_error()

    selected_port = port_var.get().strip()
    selected_baud = baud_var.get().strip()

    if not selected_port:
        set_error("Select a COM port.")
        refresh_ports()
        update_button_states()
        return

    try:
        baud_int = int(selected_baud)
    except ValueError:
        set_error("Invalid baud rate.")
        update_button_states()
        return

    refresh_ports()
    if selected_port not in port_combo["values"]:
        set_error("Selected COM port is not available.")
        update_button_states()
        return

    try:
        if ser is not None and ser.is_open and ser.port == selected_port and ser.baudrate == baud_int:
            # Already connected with the same settings
            pass
        else:
            # close existing connection if open with different settings, then open new connection
            safe_close_serial()
            ser = serial.Serial(selected_port, baud_int, timeout=0.1, write_timeout=0.1)
            time.sleep(0.2)
    except Exception as e:
        set_error(f"Open failed: {e}")
        safe_close_serial()
        refresh_ports()
        start_mode = False
        update_button_states()
        return

    # Turn active mode on
    start_mode = True
    update_button_states()
    clear_error()

    # Reset movement state tracking
    last_x_state = "STOP"
    last_y_state = "STOP"

    now = time.time()
    last_x_stop_sent_time = now
    last_y_stop_sent_time = now

    # Send stop once right when start mode begins
    try:
        send_cmd(CMD_X_STOP)
        time.sleep(0.2)
        send_cmd(CMD_Y_STOP)
    except Exception as e:
        msg = f"Write failed: {e}"
        root.after(0, lambda: handle_serial_failure(msg))
        return

def on_stop():
    # Handles the Stop button click by calling the internal_stop function to deactivate control mode and reset state, while sending the normal control command 3 times.
    internal_stop(send_normal_twice=True, error_message="")

def on_refresh_ports():
    # Handles the Refresh Ports button click by refreshing the list of available serial ports. If currently in active control mode, does not allow refreshing and shows an error instead.
    if start_mode:
        return
    refresh_ports()
    clear_error()

# Main control loop
def control_loop():
    # Main loop that runs in a separate thread to continuously read keyboard state and send commands to the STM32 based on changes. Also updates the GUI state for visual feedback. Handles serial communication errors by stopping control mode and showing an error message.
    global program_alive
    global last_x_state, last_y_state
    global last_x_stop_sent_time, last_y_stop_sent_time

    while program_alive:
        try:
            if not start_mode:
                time.sleep(0.05)
                continue

            # Only read keyboard input while start mode is on
            w_pressed = keyboard.is_pressed(KEY_Y_POS)
            a_pressed = keyboard.is_pressed(KEY_X_NEG)
            s_pressed = keyboard.is_pressed(KEY_Y_NEG)
            d_pressed = keyboard.is_pressed(KEY_X_POS)

            x_state = get_x_state()
            y_state = get_y_state()
            now = time.time()

            # If X direction changed, send the matching command
            if x_state != last_x_state:
                if x_state == "NEG":
                    send_cmd(CMD_X_NEG)
                    time.sleep(0.01)  # slight delay to prevent command flooding
                    send_cmd(CMD_X_NEG)  # send twice on direction change to increase reliability
                elif x_state == "POS":
                    send_cmd(CMD_X_POS)
                    time.sleep(0.01)  # slight delay to prevent command flooding
                    send_cmd(CMD_X_POS)  # send twice on direction change to increase reliability
                else:
                    send_cmd(CMD_X_STOP)
                    time.sleep(0.01)  # slight delay to prevent command flooding
                    send_cmd(CMD_X_STOP)
                    last_x_stop_sent_time = now
                last_x_state = x_state
                time.sleep(0.01)  # slight delay to prevent command flooding

            # While X is stopped, keep sending XS every 2 seconds
            elif x_state == "STOP" and (now - last_x_stop_sent_time) >= RESEND_STOP_INTERVAL_S:
                send_cmd(CMD_X_STOP)
                last_x_stop_sent_time = now
                time.sleep(0.01)  # slight delay to prevent command flooding

            # If Y direction changed, send the matching command
            if y_state != last_y_state:
                if y_state == "NEG":
                    send_cmd(CMD_Y_NEG)
                    time.sleep(0.01)  # slight delay to prevent command flooding
                    send_cmd(CMD_Y_NEG)  # send twice on direction change to increase reliability
                elif y_state == "POS":
                    send_cmd(CMD_Y_POS)
                    time.sleep(0.01)  # slight delay to prevent command flooding
                    send_cmd(CMD_Y_POS)  # send twice on direction change to increase reliability
                else:
                    send_cmd(CMD_Y_STOP)
                    time.sleep(0.01)  # slight delay to prevent command flooding
                    send_cmd(CMD_Y_STOP)    
                    last_y_stop_sent_time = now
                last_y_state = y_state
                time.sleep(0.01)  # slight delay to prevent command flooding


            # While Y is stopped, keep sending YS every 2 seconds
            elif y_state == "STOP" and (now - last_y_stop_sent_time) >= RESEND_STOP_INTERVAL_S:
                send_cmd(CMD_Y_STOP)
                last_y_stop_sent_time = now
                time.sleep(0.01)  # slight delay to prevent command flooding

            arrow, motion_text = get_motion_visual(x_state, y_state)

            with lock:
                gui_data["w_pressed"] = w_pressed
                gui_data["a_pressed"] = a_pressed
                gui_data["s_pressed"] = s_pressed
                gui_data["d_pressed"] = d_pressed
                gui_data["x_state"] = x_state
                gui_data["y_state"] = y_state
                gui_data["arrow"] = arrow
                gui_data["motion_text"] = motion_text

            time.sleep(POLL_TIME_S)

        except Exception as e:
            root.after(0, lambda msg=str(e): handle_serial_failure(f"Port error: {msg}"))
            safe_close_serial()
            time.sleep(0.1)


# GUI helpers

def set_key_style(widget, pressed):
    # Sets the visual style of a key widget based on whether it's pressed and whether start_mode is active. If start_mode is off, all keys are shown in a disabled style. If start_mode is on, pressed keys are shown in green and sunken, while unpressed keys are shown in gray and raised, and if both keys for a direction are pressed, they are set to red by the refresh_gui function.
    if not start_mode:
        widget.config(bg="#2a2a2a", fg="#777777", relief="raised")
        return

    if pressed:
        widget.config(bg="#33c481", fg="black", relief="sunken")
    else:
        widget.config(bg="#404040", fg="white", relief="raised")

def refresh_gui():
    # Refreshes the GUI elements based on the current state, in a thread-safe way. Shows conflicting key presses in red, updates the arrow and motion text, and displays the last sent command and any error messages.
    with lock:
        w_pressed = gui_data["w_pressed"]
        a_pressed = gui_data["a_pressed"]
        s_pressed = gui_data["s_pressed"]
        d_pressed = gui_data["d_pressed"]
        arrow = gui_data["arrow"]
        motion_text = gui_data["motion_text"]
        last_sent = gui_data["last_sent"]
        error_text = gui_data["error_text"]

    if not start_mode:
        w_pressed = a_pressed = s_pressed = d_pressed = False
        arrow = "•"
        motion_text = "Idle"

    x_conflict = a_pressed and d_pressed
    y_conflict = w_pressed and s_pressed

    # Show conflicting key presses in red
    if start_mode and y_conflict:
        w_label.config(bg="#d9534f", fg="white", relief="sunken")
        s_label.config(bg="#d9534f", fg="white", relief="sunken")
    else:
        set_key_style(w_label, w_pressed)
        set_key_style(s_label, s_pressed)

    if start_mode and x_conflict:
        a_label.config(bg="#d9534f", fg="white", relief="sunken")
        d_label.config(bg="#d9534f", fg="white", relief="sunken")
    else:
        set_key_style(a_label, a_pressed)
        set_key_style(d_label, d_pressed)

    arrow_label.config(text=arrow)
    motion_label.config(text=motion_text)
    last_sent_label.config(text=f"Last Sent: {last_sent}")
    error_label.config(text=error_text)

    if program_alive:
        root.after(30, refresh_gui)

def on_close():
    # Handles the window close event by stopping the control loop, sending the normal control command to reset the STM32, closing the serial port, and then destroying the GUI window. Sets program_alive to False to signal the control loop to exit, and uses a try-except block to ensure that even if there are errors during shutdown, the program will still attempt to close the serial port and destroy the window.
    global program_alive
    program_alive = False
    try:
        internal_stop(send_normal_twice=True, error_message="")
        safe_close_serial()
    except Exception:
        pass
    root.after(100, root.destroy)

def build_label(parent, text, fg="white", bg="#1e1e1e", font_size=11, bold=False, width=None):
    # Helper function to create a styled label with consistent font and color settings. Allows customization of text, foreground color, background color, font size, boldness, and width.
    return tk.Label(parent, text=text, font=("Arial", font_size, "bold" if bold else "normal"), fg=fg, bg=bg, width=width)

def make_key(parent, label):
    # Helper function to create a styled label that represents a keyboard key in the GUI, with a consistent size and font. The label parameter specifies the text shown on the key (e.g., "W", "A", "S", "D").
    return tk.Label(parent, text=label, width=6, height=3, font=("Arial", 16, "bold"), relief="raised", bd=4, bg="#404040", fg="white")

def build_combo(parent, variable, state, values, width=12):
    # Helper function to create a styled ttk Combobox with consistent font and color settings. Allows customization of the associated variable, state (e.g., "readonly"), list of values, and width.
    return ttk.Combobox(parent, textvariable=variable, state=state, width=width, values=values)

def build_button(parent, text, command, width=10, font_size=11, bg="#33c481", fg="black", state="normal"):
    # Helper function to create a styled button with consistent font and color settings. Allows customization of the button text, command callback, width, font size, background color, foreground color, and state (e.g., "normal", "disabled").
    return tk.Button(parent, text=text, command=command, width=width, font=("Arial", font_size, "bold"), bg=bg, fg=fg, state=state)

def build_gui(root):
    # Builds the GUI layout and initializes all widgets. Sets up the main window, creates frames for organization, and places labels, buttons, and other controls in a visually appealing way. Also starts the control loop thread and sets up the protocol handler for window closing. Uses the helper functions to create styled widgets and organizes them into a coherent interface for controlling the STM32 with the keyboard.
    global port_var, baud_var
    global port_combo, baud_combo
    global refresh_ports_button, start_button, stop_button
    global mode_label, error_label
    global w_label, a_label, s_label, d_label
    global arrow_label, motion_label, last_sent_label
    # GUI initialization
    root = root
    root.title("STM32 Keyboard Motion Control")
    root.geometry("780x500")
    root.configure(bg="#1e1e1e")
    root.resizable(False, False)

    # GUI Layout
    title_label = build_label(root, "STM32 Keyboard Motion Control", font_size=18, bold=True, fg="#4da6ff")
    title_label.pack(pady=10)

    # Top controls frame for COM port, baud rate, and start/stop buttons
    top_controls = tk.Frame(root, bg="#1e1e1e")
    top_controls.pack(pady=6)

    build_label(top_controls, "COM Port:", bg="#1e1e1e", fg="white", font_size=11, bold=True).grid(row=0, column=0, padx=6)

    port_var = tk.StringVar()
    port_combo = build_combo(top_controls, port_var, state="readonly", values=list_available_ports())
    port_combo.grid(row=0, column=1, padx=6)

    build_label(top_controls, "Baud:", bg="#1e1e1e", fg="white", font_size=11, bold=True).grid(row=0, column=2, padx=6)

    baud_var = tk.StringVar(value=DEFAULT_SERIAL_BAUDRATE)
    baud_combo = build_combo(top_controls, baud_var, state="readonly", width=12, values=BAUD_OPTIONS)
    baud_combo.grid(row=0, column=3, padx=6)

    baud_combo.grid(row=0, column=3, padx=6)

    refresh_ports_button = build_button(top_controls, text="Refresh Ports", command=on_refresh_ports, width=12, font_size=10, bg="#4da6ff", fg="black")
    refresh_ports_button.grid(row=0, column=4, padx=8)

    start_button = build_button(top_controls, text="Start", command=on_start, width=10, font_size=11, bg="#33c481", fg="black")
    start_button.grid(row=0, column=5, padx=8)

    stop_button = build_button(top_controls, text="Stop", command=on_stop, width=10, font_size=11, bg="#555555", fg="#cccccc", state="disabled")
    stop_button.grid(row=0, column=6, padx=8)

    # Mode and error display labels
    mode_label = build_label(root, "Mode: IDLE", fg="#aaaaaa", font_size=12, bold=True)
    mode_label.pack()

    error_label = build_label(root, "", fg="#d9534f", font_size=10, bold=False)
    error_label.pack(pady=(2, 6))

    # Main frame for keyboard visualization and motion display
    main_frame = tk.Frame(root, bg="#1e1e1e")
    main_frame.pack(fill="both", expand=True, padx=20, pady=10)

    left_frame = tk.Frame(main_frame, bg="#1e1e1e")
    left_frame.pack(side="left", padx=30)

    right_frame = tk.Frame(main_frame, bg="#1e1e1e")
    right_frame.pack(side="left", padx=50)

    keyboard_frame = tk.Frame(left_frame, bg="#1e1e1e")
    keyboard_frame.pack(pady=10)

    spacer1 = build_label(keyboard_frame, "", bg="#1e1e1e", width=6)
    w_label = make_key(keyboard_frame, "W")
    spacer2 = build_label(keyboard_frame, "", bg="#1e1e1e", width=6)

    a_label = make_key(keyboard_frame, "A")
    s_label = make_key(keyboard_frame, "S")
    d_label = make_key(keyboard_frame, "D")

    spacer1.grid(row=0, column=0, padx=6, pady=6)
    w_label.grid(row=0, column=1, padx=6, pady=6)
    spacer2.grid(row=0, column=2, padx=6, pady=6)

    a_label.grid(row=1, column=0, padx=6, pady=6)
    s_label.grid(row=1, column=1, padx=6, pady=6)
    d_label.grid(row=1, column=2, padx=6, pady=6)

    legend = build_label(left_frame, "WASD enabled only in Start mode", fg="#cfcfcf", bg="#1e1e1e", font_size=11)
    legend.pack(pady=10)

    arrow_label = build_label(right_frame,text="•", font_size=90, fg="#4da6ff",bg="#1e1e1e", bold=True)
    arrow_label.pack(pady=(20, 10))

    motion_label = build_label(right_frame, text="Idle", font_size=16, fg="#cfcfcf", bg="#1e1e1e", bold=True)
    motion_label.pack(pady=10)

    last_sent_label = build_label(right_frame, text="Last Sent: None", font_size=13, fg="#d9d9d9", bg="#1e1e1e")
    last_sent_label.pack(pady=10)

    status_label = build_label(right_frame, text="Select COM and baud, then press Start", font_size=11, fg="#aaaaaa", bg="#1e1e1e")
    status_label.pack(pady=10)

    # Initial setup
    refresh_ports()
    if not baud_var.get():
        baud_var.set(DEFAULT_SERIAL_BAUDRATE)
    update_button_states()
    enter_idle_visual_state()
    
    # Start the control loop in a separate thread to continuously read keyboard input and send commands without blocking the GUI. The thread is set as a daemon so it will automatically exit when the main program exits.
    control_thread = threading.Thread(target=control_loop, daemon=True)
    control_thread.start()

    refresh_gui()

    # Set up the protocol handler for when the window is closed to ensure that the control loop is stopped, the serial port is closed, and the program exits cleanly.
    root.protocol("WM_DELETE_WINDOW", on_close)

# Main window setup

if __name__ == "__main__":
    # Declare global variables for GUI elements that will be initialized in build_gui and accessed in other functions. Then create the main Tkinter window, build the GUI, and start the main event loop.
    root = tk.Tk()
    build_gui(root)

    root.mainloop()