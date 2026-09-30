# V-Plotter

A vertical plotter that draws vector images on vertical surfaces using two stepper motors. The system combines a custom PCB, ATmega328 firmware that interprets HPGL commands, and a Python desktop interface to send jobs and control the drawing.

Developed for *Taller de Sistemas Digitales* — Electronic Engineering, Universidad Nacional de La Plata (UNLP), 2025.

![V-Plotter drawing](docs/media/plotter.jpg)<img width="1280" height="960" alt="dibujo vplotter" src="https://github.com/user-attachments/assets/1422a5d6-73fb-4976-a607-f4437e978ddb" />






## Demo

(https://github.com/user-attachments/assets/00215eca-22d8-4fac-bc4f-44b1913aaaf5)
## How it works

```
[ Python interface ] --serial--> [ ATmega328 firmware ] --> [ Stepper drivers ] --> [ Motors / pen ]
   loads vector image,             parses HPGL,
   sends HPGL commands             computes cable lengths
```

1. The **interface** loads a vector image, converts it to HPGL commands and streams them to the board.
2. The **firmware** interprets each HPGL command and translates the pen position into the lengths of the two cables that hold it.
3. The **stepper motors** wind or unwind the cables to move the pen across the surface.

## Repository structure

| Folder | Content |
|---|---|
| [`firmware/`](firmware/) | ATmega328 firmware: HPGL parser and stepper motor control |
| [`interface/`](interface/) | Python interface to send drawings and control the plotter |

## Hardware

- **MCU:** ATmega328
- **Actuators:** two stepper motors and pen lift mechanism
- **PCB:** designed in Altium Designer, fabricated in-house by chemical etching and hand-assembled

<img width="1132" height="727" alt="image" src="https://github.com/user-attachments/assets/10d169cd-4431-473b-818e-f0781be9a52d" />

| Layout | Finished board |
|---|---|
| <img width="646" height="560" alt="image" src="https://github.com/user-attachments/assets/151aabf8-ed4c-4f02-90f6-f78642c79565" />


## Team

| Member | Contribution |
|---|---|
| **Juan Lucas Gutiérrez** — [@gutierrez-lucas](https://github.com/gutierrez-lucas) | Firmware |
| **Juan F. Baleztena** — [@jfbaleztena](https://github.com/jfbaleztena) | Python interface, PCB design in Altium, board fabrication and assembly |

> This repository combines the project's original firmware and interface repositories, preserving their full commit history.
