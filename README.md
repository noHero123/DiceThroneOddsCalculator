## dice throne odds calculator
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