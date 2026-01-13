# Assembly Language Learning – RISC-V Memory Game Project

A low-level systems project implementing an interactive **memory card matching game** using **RISC-V assembly**, running on a custom embedded environment with **VGA graphics output** and **PS/2 keyboard input**.

This repository demonstrates deep understanding of **computer architecture, low-level programming, and hardware–software interaction**.

---

## 📌 Project Overview

This project was built as part of a computer engineering coursework focused on low-level system programming. The core deliverable is a fully functional **memory matching game**, written primarily in **RISC-V assembly**, running on an embedded system with:

* VGA display output
* PS/2 keyboard input
* Memory-mapped I/O
* Custom runtime environment

Unlike typical high-level projects, this work required reasoning about **registers, memory layout, control flow, and hardware interfaces** directly.

---

## 🎮 Game Features

* Interactive memory card matching gameplay
* Real-time VGA graphical output
* Keyboard-controlled navigation (WASD / arrow keys)
* Card flipping, matching logic, and game state tracking
* Score and state updates rendered to screen
* Deterministic behavior with low-level control over execution

---

## 🧠 System Architecture

The system operates on a bare-metal style environment:

* **RISC-V Assembly Layer**

  * Core game logic
  * Input handling
  * State machine for gameplay
  * Memory management
  * Low-level control flow

* **Hardware Interface**

  * VGA display accessed via memory-mapped I/O
  * PS/2 keyboard input via I/O registers
  * Direct register manipulation for device control

This structure closely mirrors how firmware and embedded systems are built in real-world environments.

---

## 🛠 Technical Skills Demonstrated

* **Languages**

  * RISC-V Assembly
  * C (supporting components where applicable)

* **Core Topics**

  * Computer architecture
  * Instruction-level programming
  * Memory-mapped I/O
  * Register-level debugging
  * Finite state machines
  * Embedded system constraints
  * Low-level performance reasoning

* **Engineering Skills**

  * Careful debugging without high-level tooling
  * Precise control over program execution
  * Reasoning about stack, heap, and registers
  * Translating high-level logic into assembly

---

## 📈 Key Contributions

* Designed full game logic in RISC-V assembly
* Implemented keyboard-driven navigation and input handling
* Built rendering pipeline via VGA memory-mapped interface
* Developed robust state machine for gameplay transitions
* Debugged low-level issues involving registers, timing, and memory
* Validated functionality on real/simulated hardware environment

---

## 🎯 Why This Project Matters

This project demonstrates skills rarely shown in typical student portfolios:

* Comfort working **below the operating system level**
* Strong understanding of **how software maps to hardware**
* Ability to debug without relying on high-level abstractions
* Experience similar to:

  * Firmware development
  * Embedded systems
  * OS kernel work
  * Hardware bring-up

It clearly shows the ability to handle complexity and think at the system level.

---

## 📎 Note

This project was developed for educational purposes and reflects practical training in low-level programming, computer architecture, and embedded systems design.
