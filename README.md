# mAInesweeper Editor

An Unreal Engine plugin that adds a custom tab to play Minesweeper. The game board is generated using AI (Gemini).

![Minesweeper_Splash](Images/Minesweeper_Splash.png)

# How to Run

1. **Download project**
2. Double-click on **mAInesweeper.uproject**
3. Setup AI Api Key:
   - Go to **Project Settings** > **AI Api Settings**
   - Enter a valid **Gemini api key**
4. **Play :)**

# Features

- **Toolbar Integration**: Adds a button to the Level Editor Toolbar to open the Minesweeper Tab
- **Minesweeper Tab** includes:
  - A **"Play Again"** button
  - **Match statistics** (number of bombs generated)
  - An interactive board with **clickable tiles**
  - A **chat-like prompt** for interacting with Gemini AI, specialized in generating Minesweeper boards
