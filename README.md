# StateFlow
DFA and NFA simulator

## Building
Clone the repo and initialize the submodules
```sh
git clone --recurse-submodules https://github.com/kshku/StateFlow.git
cd StateFlow
```
or
```sh
git clone https://github.com/kshku/StateFlow.git
cd StateFlow
git submodule update --init --recursive
```
Build using CMake
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 8
```
To run
```sh
./build/stateflow
```

<!-- ## How to use  
Choose DFA or NFA from the main menu, or you can also load a previously saved state machine.  
One DFA and one NFA examples which accepts strings ending with ab or ba are saved as `end_ab.fsm` and `end_ab_nfa.fsm` in the project root directory itself.  

In the editor, you can specify the alphabet (each character including space is considered as letter in alphabet).

Transition button can be used to switch to Transition mode to edit the transition lines. The editor starts in the Node mode, in which you can edit the nodes.  

When done with the FSM you can click on the Simulate button, which validates the given FSM and reports the error if any, otherwise takes you to the animation screen.

Use save button to save the current FSM you are editing (make sure that the file ends with .fsm since when loading the file we only look for .fsm type files).

Left click or Middle click and drag to navigate around the canvas. You can also use wasd keys, arrow keys or hjkl keys for navigation. Scroll to zoom in and zoom out.

In Node mode, right click to add new node.  
By selecting a node, you can edit the name, and specify whether it is starting node or accepting node. Also you can move the node around once you have selected it by clicking and dragging it. You can select the node and press delete to delete the node.

In Transition mode, to add transitions, click on the From box and select the node, give the input on which the transtion takes place, and click on the To box and select the node. Add button will appear which should be clicked to add the transition. You can select transition and press delete to delete the node.  
NOTE: Similar to alphabets, each character is considered as one input (multiple characters can be there in single transition line).

Can press Backspace to go back to the main menu from the editor or to the editor from animation.

In animation, give the input and press the Start animation button to animate the FSM! -->

## How to Use  
From the **main menu**, choose DFA or NFA, or load a previously saved state machine.  
Two example files are provided in the project root:  
- `end_ab.fsm` – DFA example that accepts strings ending with `ab`  
- `end_ab_nfa.fsm` – NFA example that accepts strings ending with `ba`  

### Editor  
- **Alphabet**: you can specify the alphabet. Each character (including space) is treated as one letter.  
- **Modes**:  
  - **Node Mode** (default) – create and edit nodes.  
  - **Transition Mode** – create and edit transition lines.  

#### Navigation  
- Left-click or Middle-click + drag → pan around the canvas  
- Scroll → zoom in/out  
- Keys: `WASD`, arrow keys, or `HJKL` → pan around the canvas  

#### Node Mode  
- Right-click → add new node  
- Select a node to:  
  - Rename it  
  - Mark it as **start** or **accepting**  
  - Drag to reposition it  
- Press **Delete** → delete selected node  

#### Transition Mode  
1. Click **From** box → select starting node  
2. Enter input (each character is one symbol; multiple characters allowed per transition)  
3. Click **To** box → select target node  
4. Click **Add** button → add transition  
- Press **Delete** → delete selected transition line  

### Simulation  
- Click **Simulate** to validate the FSM.  
  - If errors exist, they will be reported.  
  - If valid, the app switches to the animation screen.  
- **Backspace** → return to editor (from animation) or to main menu (from editor).  

### Saving & Loading  
- Click **Save** to save the current FSM (file must end with `.fsm`).  
- Use **Load** to open an existing `.fsm` file.  

### Animation  
- Enter input string in the text box.  
- Click **Start Animation** to animate the FSM step-by-step.  


### Quick Reference  
- **Backspace** → go back  
- **Delete** → delete selected node/transition  
- **Right-click** → add new node  
- **Middle/Left-drag** → move around canvas  
- **Scroll** → zoom in/out  
