==============================================================================================
		Instrucciones para Ejecutar la Interfaz y el Simulador del V-Plotter
==============================================================================================

Esta guía explica cómo preparar el entorno en Linux para ejecutar la Interfaz de Usuario y el Simulador del Plotter de forma conjunta. Se utiliza Python, junto con algunos módulos externos, y se configuran puertos serie virtuales para simular la comunicación entre ambos programas.

Requisitos

Software y Librerías

Asegurate de tener instalado lo siguiente:

	Python 3, pip3 (administrador de paquetes de Python) y socat (para crear puertos serie virtuales)

Librerías Python
Ejecutá el siguiente comando para instalar las librerías necesarias:

	pip3 install pyserial svgpathtools pygame


Además, para utilizar Tkinter (la librería de interfaces gráficas de Python), en la mayoría de las distribuciones de Linux se instala mediante:

	sudo apt install python3-tk







==============================================================================================
==============================================================================================

		Configuración de Puertos Serie Virtuales
==============================================================================================

Para simular la comunicación entre la Interfaz de Usuario y el Simulador del Plotter, se utilizarán puertos serie virtuales. Utilizaremos socat para crearlos:

Instalar socat (si aún no lo tenés):

sudo apt update
sudo apt install socat


Crear el par de puertos virtuales:
Ejecutá en una terminal:

socat -d -d pty,raw,echo=0 pty,raw,echo=0

2025/04/07 12:00:00 socat[1234] N PTY is /dev/pts/3
2025/04/07 12:00:00 socat[1234] N PTY is /dev/pts/4
2025/04/07 12:00:00 socat[1234] N starting data transfer loop with FDs [5,5] and [7,7]
En este ejemplo, se han creado dos puertos: /dev/pts/3 y /dev/pts/4.

Asignación de puertos:

Simulador del Plotter: Configurá el script para usar el puerto /dev/pts/3

Interfaz de Usuario: Configurá el script para usar el puerto /dev/pts/4






==============================================================================================
==============================================================================================

		Ejecución de los Programas
==============================================================================================
Simulador del Plotter
El simulador (que utiliza pygame) abre una ventana con fondo negro simulando una pizarra (80x60 cm a escala). Se encarga de recibir comandos serie, dibujar los trazos en tiempo real y responder con OK.

Asegurate de que en el script del simulador (por ejemplo, simulador_plotter.py) la variable de puerto esté configurada correctamente:

PUERTO = "/dev/pts/3"
Ejecutá el simulador:

python3 simulador_plotter.py


Interfaz de Usuario
La interfaz (con Tkinter) permite:

	Seleccionar un archivo SVG.	
	Convertirlo a HPGL (usando Inkscape) y guardarlo en la misma carpeta.
	Procesar el SVG, traducir los trazos a comandos G-code (ej.: G1 Xnn Ynn).
	Enviar cada comando por serie y esperar un OK antes de enviar el siguiente.
	Mostrar un log en tiempo real con los comandos enviados y las respuestas recibidas.
	Asegurate de que en el script de la interfaz (por ejemplo, plotter_interface.py) la variable de puerto esté configurada a:

		PUERTO = "/dev/pts/4"


Ejecutá la interfaz:

	python3 plotter_interface.py




==============================================================================================
==============================================================================================
		
		Notas Adicionales
==============================================================================================
Inkscape:

The most recent version (slightly newer than what is available directly from Ubuntu) is available from our stable PPA for Ubuntu and derivative Linux distributions:

sudo add-apt-repository universe
sudo add-apt-repository ppa:inkscape.dev/stable
sudo apt-get update
sudo apt install inkscape


Asegurate de tener Inkscape instalado y que el comando inkscape esté disponible en tu PATH, ya que el script usa Inkscape para convertir SVG a HPGL. En Ubuntu o Debian, podés instalarlo con:

	sudo apt install inkscape

Permisos del Puerto Serie Virtual:
Los dispositivos creados por socat se gestionan como ptys y, en general, no requieren configuración adicional de permisos.

Logs y Depuración:
La interfaz muestra en un widget de log todos los comandos enviados y las respuestas recibidas. Esto te ayudará a depurar y ajustar el flujo de comunicación.
