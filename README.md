## Dice Throne Odds Calculator
thanks to Assassinater for his python gui.<br />
to build from source you need boost, eigen and sqlite3.<br /><br />
contains lots of unrecommended/old code, just to show 4 different ways to calculate the odds:
- simulations of ORP (Offensive Roll Phase) (extremly optimized, almost 15 mio simulations/sec)
- MCTS (Monte Carlo Tree Search)
- brute force
- markov chains (inspired from https://math.stackexchange.com/questions/4032313/probability-of-yahtzee-straight-with-strict-re-roll-rules) (currently preffered way)

Python Gui:
![python gui](/assets/images/GUI.jpg)

numbers in brackets are the dice you have to reroll for maximized odds.

### Installation:
- download latest release from: https://github.com/noHero123/DiceThroneOddsCalculator/releases and extract it.
- (optional) if you want to use the odds calculator for DTA, also download the precomputed database from https://github.com/noHero123/DiceThroneOddsCalculator/releases/tag/1.0.0_DTA and extract it to the same folder like the stuff from first step.
- start odds_calc.exe


### compile with linux
-install eigen + sqlite:
sudo apt-get install libeigen3-dev
sudo apt-get install libsqlite3-dev

