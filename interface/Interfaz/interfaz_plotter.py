import tkinter as tk
from tkinter import filedialog, scrolledtext, messagebox
import os
import serial
import time
import threading
import subprocess
from svgpathtools import svg2paths

# Configuración del puerto y velocidad\ nPUERTO = "/dev/pts/3"  # Ajustar según socat
BAUDIOS = 9600
PUERTO = "/dev/pts/5"  # Ajustar según socat
ESCALA = 10  # Escala para convertir SVG a HPGL

class InterfazPlotter:
    def __init__(self, master):
        self.master = master
        master.title("V-Plotter Interface")

        # Lista de comandos HPGL pendientes por enviar
        self.comandos_hpgl = []

        # Variable para modo de conversión
        self.modo_conv = tk.StringVar(value="svgpathtools")

        # Botones de selección de método de conversión
        frame_mode = tk.LabelFrame(master, text="Select conversion method")
        frame_mode.pack(pady=5)
        tk.Radiobutton(frame_mode, text="svgpathtools", variable=self.modo_conv,
                       value="svgpathtools").pack(side=tk.LEFT, padx=10)
        tk.Radiobutton(frame_mode, text="inkscape", variable=self.modo_conv,
                       value="inkscape").pack(side=tk.LEFT, padx=10)

        # Botón "Avanzar": envía un comando HPGL a la vez
        self.btn_avanzar = tk.Button(master, text="Avanzar", state="disabled",
                                     command=self.enviar_siguiente_comando)
        self.btn_avanzar.pack(pady=5)

        # Botón para cargar SVG
        self.cargar_btn = tk.Button(master, text="Load SVG", command=self.cargar_svg)
        self.cargar_btn.pack(pady=10)

        # Área de log
        self.log_text = scrolledtext.ScrolledText(master, width=80, height=15, state='disabled')
        self.log_text.pack(pady=10)

        # Label de estado
        self.status = tk.Label(master, text="Waiting for file...", fg="blue")
        self.status.pack(pady=5)

        # Inicializar conexión serial
        try:
            self.ser = serial.Serial(PUERTO, BAUDIOS, timeout=1)
            time.sleep(2)
            self.log(f"✅ Connected to port {PUERTO}")
        except Exception as e:
            messagebox.showerror("Error", f"Could not open port {PUERTO}.\n{e}")
            master.destroy()

    def log(self, mensaje):
        """Agrega un mensaje al log y mantiene el scroll abajo."""
        self.log_text.config(state='normal')
        self.log_text.insert(tk.END, mensaje + "\n")
        self.log_text.see(tk.END)
        self.log_text.config(state='disabled')

    def cargar_svg(self):
        archivo = filedialog.askopenfilename(filetypes=[("SVG Files", "*.svg")])
        if not archivo:
            return
        self.status.config(text="Processing SVG...")
        threading.Thread(target=self.procesar_svg, args=(archivo,), daemon=True).start()

    def procesar_svg(self, archivo):
        metodo = self.modo_conv.get()
        self.log(f"Using conversion method: {metodo}")
        hpgl_file = self.convertir_svg_a_hpgl(archivo, metodo)
        if hpgl_file is None:
            self.status.config(text="Conversion failed")
            return
        # Carga los comandos para envío paso a paso
        self.enviar_hpgl(hpgl_file)

    def convertir_svg_a_hpgl(self, svg_path, metodo):
        """Selector de método de conversión: inkscape o svgpathtools."""
        if metodo == "inkscape":
            return self.convertir_con_inkscape(svg_path)
        else:
            return self.convertir_con_svgpathtools(svg_path)

    def convertir_con_svgpathtools(self, svg_path):
        """Convierte SVG a HPGL directamente con svgpathtools."""
        directorio = os.path.dirname(svg_path)
        nombre_base = os.path.splitext(os.path.basename(svg_path))[0]
        hpgl_path = os.path.join(directorio, nombre_base + ".hpgl")
        try:
            paths, _ = svg2paths(svg_path)
            cmds = ["IN", "SP1"]
            for path in paths:
                for seg in path:
                    x1 = int(seg.start.real * ESCALA)
                    y1 = int(seg.start.imag * ESCALA)
                    x2 = int(seg.end.real * ESCALA)
                    y2 = int(seg.end.imag * ESCALA)
                    cmds.append(f"PU{x1},{y1}")
                    cmds.append(f"PD{x2},{y2}")
            cmds.extend(["SP0", "PU0,0"] )
            with open(hpgl_path, "w") as f:
                f.write(";".join(cmds) + ";")
            self.log("SVG convertido a HPGL (svgpathtools): " + hpgl_path)
            return hpgl_path
        except Exception as e:
            self.log("❌ Error generando HPGL: " + str(e))
            return None

    def convertir_con_inkscape(self, svg_path):
        """Convierte SVG a HPGL usando Inkscape."""
        directorio = os.path.dirname(svg_path)
        nombre_base = os.path.splitext(os.path.basename(svg_path))[0]
        hpgl_path = os.path.join(directorio, nombre_base + ".hpgl")
        try:
            subprocess.run([
                "inkscape", svg_path,
                "--export-type=hpgl",
                "--export-filename", hpgl_path
            ], check=True)
            self.log("SVG convertido a HPGL (Inkscape): " + hpgl_path)
            return hpgl_path
        except subprocess.CalledProcessError as e:
            self.log("❌ Error converting SVG to HPGL (Inkscape): " + str(e))
            return None

    def enviar_hpgl(self, hpgl_path):
        """Carga los comandos HPGL en memoria y habilita el botón Avanzar."""
        try:
            with open(hpgl_path, "r") as f:
                lineas = f.read().split(";")
            # Construye lista de comandos terminados en ';'
            self.comandos_hpgl = [l.strip() + ";" for l in lineas if l.strip()]
            self.log(f"HPGL cargado con {len(self.comandos_hpgl)} comandos.")
            self.status.config(text="Waiting for user to send each command...")
            self.btn_avanzar.config(state="normal")
        except Exception as e:
            self.log("❌ Error loading HPGL: " + str(e))

    def enviar_siguiente_comando(self):
        """Envía un único comando cuando el usuario pulsa el botón."""
        if not self.comandos_hpgl:
            self.log("✅ All commands sent.")
            self.status.config(text="Completed")
            self.btn_avanzar.config(state="disabled")
            return
        cmd = self.comandos_hpgl.pop(0)
        self.log(f"Sending: {cmd}")
        self.ser.write((cmd + "\n").encode())
        # Espera respuesta ACK/NACK\ nresp = b""
        timeout = time.time() + 5
        while not resp.endswith(b"ACK\n") and not resp.endswith(b"NACK\n"):
            if time.time() > timeout:
                self.log("Timeout waiting for ACK after: " + cmd)
                break
            byte = self.ser.read(1)
            if byte:
                resp += byte
            else:
                time.sleep(0.05)
        recibido = resp.decode().strip()
        self.log("Received: " + recibido)
        if recibido == "NACK":
            self.log("⚠️ ERROR: Comando rechazado")
            self.btn_avanzar.config(state="disabled")

# Ejecutar interfaz
if __name__ == "__main__":
    root = tk.Tk()
    app = InterfazPlotter(root)
    root.mainloop()
