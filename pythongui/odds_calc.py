import random
import PySimpleGUI as sg
import pickle
import subprocess

fontsize = 20

simcount = 5000

curentdicevalues = "12345"

rollatmps = 2

rerolls = 0

wilds = 0

sixit = False

samesies = False

currenthero = "default"
mydiceanatomy = ["A", "A", "A", "B", "B", "C"]
abilitynames = ["ability"]
rawrollobjs = ["AAA"]
abilitylocations = ["A1"]

smallname = "a name"
smalllocation = "C1"
largename = "a name"
largelocation = "C5"

allabilitynames = abilitynames.copy()
allabilitynames.extend([smallname, largename])

alllocations = abilitylocations.copy()
alllocations.extend([smalllocation, largelocation])

chaseability = "A1"


def raw_to_obj(rawobjs):
   newrollobjs = []
   for i in rawobjs:
       temp = []
       for j in i:
           temp.append(j)
       newrollobjs.append(temp)
   return newrollobjs


rollobjs = raw_to_obj(rawrollobjs)

dice = []

smst = [[1, 2, 3, 4], [2, 3, 4, 5], [3, 4, 5, 6]]
lgst = [[1, 2, 3, 4, 5], [2, 3, 4, 5, 6]]

custname = "Name"
custdiceanat = ["A","A","A","A","A","A"]
custabilnames = [""]
custrollobjs = ["ZZZ"]
custabilitylocate = ["A1"]
custsmname = "Small Name"
custsmlocate = "C1"
custlgname = "Large Name"
custlglocate = "C5"
custdefodds = [98]

def reset_custom():
   global custname
   global custname
   global custdiceanat
   global custabilnames
   global custrollobjs
   global custabilitylocate
   global custsmname
   global custsmlocate
   global custlgname
   global custlglocate
   global custdefodds
   custname = "Name"
   custdiceanat = ["A", "A", "A", "A", "A", "A"]
   custabilnames = [""]
   custrollobjs = ["ZZZ"]
   custabilitylocate = ["A1"]
   custsmname = "Small Name"
   custsmlocate = "C1"
   custlgname = "Large Name"
   custlglocate = "C5"
   custdefodds = [98]

everylocation = ['A1','A2','A3','A4','A5','A6','A7','A8',
                'B1','B2','B3','B4','B5','B6','B7','B8',
                'C1','C2','C3','C4','C5','C6','C7','C8',
                'D1','D2','D3','D4','D5','D6','D7','D8',
                'E']


class Hero:
   def __init__(self, name, diceanat, abilnames, rollobjectives, abilitylocate, smname, smlocate, lgname, lglocate, defodds=0):
       self.name = name
       self.diceanat = diceanat
       self.abilnames = abilnames
       self.rollobjectives = rollobjectives
       self.abilitylocate = abilitylocate
       self.smname = smname
       self.smlocate = smlocate
       self.lgname = lgname
       self.lglocate = lglocate
       self.defaultodds = defodds

   def set_as_active(self):
       global currenthero
       global mydiceanatomy
       global abilitynames
       global rawrollobjs
       global abilitylocations
       global smallname
       global smalllocation
       global largename
       global largelocation
       global allabilitynames
       global alllocations
       global rollobjs
       global hero
       hero = self
       currenthero = self.name
       mydiceanatomy = self.diceanat
       abilitynames = self.abilnames
       rawrollobjs = self.rollobjectives
       abilitylocations = self.abilitylocate
       smallname = self.smname
       smalllocation = self.smlocate
       largename = self.lgname
       largelocation = self.lglocate
       allabilitynames = self.abilnames.copy()
       allabilitynames.extend([self.smname, self.lgname])
       alllocations = self.abilitylocate.copy()
       alllocations.extend([self.smlocate, self.lglocate])
       rollobjs = []
       for thisrollobj in rawrollobjs:
           temp = []
           for letter in thisrollobj:
               temp.append(letter)
           rollobjs.append(temp)
       if isinstance(self.defaultodds, int):
           defaultdict = dict.fromkeys(allabilitynames, self.defaultodds)
       else:
           defaultdict = dict(zip(allabilitynames, self.defaultodds))
       print_results(defaultdict)
       window1['-HERONAME-'].update(currenthero)
       window1['-CHASE-'].update(abilitynames[0])
       window1['-DISPLAYING-'].update('Default Odds')


Artificer = Hero("Artificer",
                ["A", "A", "A", "B", "B", "C"],
                ["SPANNER STRIKE 1", "SPANNER STRIKE 2", "SPANNER STRIKE 3", "SCHEMATICS", "EUREKA", "OVERCLOCK",
                 "MAXIMUM POWER!", "NUTS N BOLTS", "SCRATCH BUILT", "POWER UP", "SWARMING BOTS"],
                ["AAA", "AAAA", "AAAAA", "BBBC", "AABBB", "CCCC", "CCCCC", "AAAC", "AABB", "CCC", "ABBC"],
                ["A1", "A2", "A3", "B1", "B5", "D1", "E", "C3", "B7", "D3", "C7"],
                "JOLT",
                "C1",
                "ZAPPPP!",
                "C5",
                [98,88,51,60,49,10,1,77,87,35,72,60,24])

Huntress = Hero("Huntress",
               ["A", "A", "B", "B", "C", "D"],
               ["ANIMALISTIC 1", "ANIMALISTIC 2", "ANIMALISTIC 3", "SAVAGE", "RESUSCITATE", "FERAL INSTINCTS", "FERAL",
                "PREDATORY ADVANCE", "JUNGLE FURY!", "FEROCIOUS", "JUGULAR", "SWIPE", "HUNT"],
               ["AAA", "AAAA", "AAAAA", "BBBD", "CC", "AAAC", "DDDD", "AABBC", "DDDDD", "DDD", "ABD", "AAD", "BBD"],
               ["A1", "A2", "A3", "B1", "A5", "B5", "D1", "C5", "E", "D3", "C7", "B7", "B3"],
               "ONSLAUGHT (small)",
               "C1",
               "ONSLAUGHT (large)",
               "C3",
               [84,54,17,60,70,60,10,42,1,35,85,82,82,60,24])

Monk = Hero("Monk",
           ["A", "A", "B", "C", "C", "D"],
           ["FIST STRIKE 1", "FIST STRIKE 2", "FIST STRIKE 3", "MEDITATE", "COMBO STRIKE", "TEMPEST RUSH",
            "LOTUS STRIKE", "TRANSCENDENCE!", "WAY OF THE LOTUS", "ZEN STRIKE", "WAY OF THE MONK"],
           ["AAA", "AAAA", "AAAAA", "CCC", "AAAB", "BBB", "DDDD", "DDDDD", "DDD", "AACC", "ABCD"],
           ["A1", "A2", "A3", "B1", "A5", "B5", "D1", "E", "D3", "B3", "C7"],
           "FIST OF HARMONY",
           "C1",
           "FIST OF TRANQUILITY",
           "C5",
           [84,54,17,84,60,36,10,1,35,78,63,60,24])

Paladin = Hero("Paladin",
              ["A", "A", "B", "B", "C", "D"],
              ["HOLY LIGHT", "RETALIATE", "RIGHTEOUS COMBAT", "MIGHTY PRAYER", "RIGHTEOUS PRAYER", "RESOLUTE FAITH!",
               "OFFENSIVE STANCE", "STEADFAST", "REQUITAL", "PROSPERITY"],
              ["CC", "BBBD", "AAABB", "AAAD", "DDDD", "DDDDD", "AAC", "AAB", "ABCD", "DDD"],
              ["C5", "B1", "A5", "B5", "D1", "E", "B7", "A7", "B3", "D3"],
              "HOLY ATTACK (small)",
              "C1",
              "HOLY ATTACK (large)",
              "C3",
              [70,60,41,60,10,1,82,93,62,35,60,24])

CursedPirate = Hero("Cursed Pirate",
                   ["A", "A", "A", "B", "B", "C"],
                   ["CUTLASS STRIKE 1", "CUTLASS STRIKE 2", "CUTLASS STRIKE 3", "X MARKS THE SPOT", "WALK THE PLANK",
                    "BLACK SPOT", "SHIVER ME TIMBERS", "PIRATES TOUCH!"],
                   ["AAA", "AAAA", "AAAAA", "BBB", "ABBC", "CCCC", "ACCC", "CCCCC"],
                   ["A1", "A2", "A3", "B1", "B5", "D1", "C5", "E"],
                   "LIGHT THE FUSE (small)",
                   "C1",
                   "LIGHT THE FUSE (large)",
                   "C3",
                   [98,88,51,84,72,10,24,1,60,24])

Samurai = Hero("Samurai",
              ["A", "A", "A", "B", "B", "C"],
              ["KATANA SLICE 1", "KATANA SLICE 2", "KATANA SLICE 3", "WAKIZASHI", "SOLEMNITY", "HAGAKURE", "SHOGUN!",
               "EMPOWER", "HAGA", "SOLEMN"],
              ["AAA", "AAAA", "AAAAA", "AACC", "AABBB", "CCCC", "CCCCC", "ABC", "CCC", "BBB"],
              ["A1", "A2", "A3", "B1", "B5", "D1", "E", "C7", "D3", "B7"],
              "BUDO",
              "C1",
              "MASAMUNE",
              "C5",
              [98,88,51,49,49,10,1,86,35,84,60,24])

Seraph = Hero("Seraph",
             ["A", "A", "A", "B", "C", "D"],
             ["HOLY BLADE 1", "HOLY BLADE 2", "HOLY BLADE 3", "GLORIOUS", "HOLY SMITE", "PURIFY", "HIGHEST POWER",
              "SPLIT THE HEAVENS!", "HEAVENS BLESSING", "TAKE FLIGHT", "CHERUBIM", "HOLY COMMAND", "DIVINE VISAGE"],
             ["AAA", "AAAA", "AAAAA", "AAAB", "ABCD", "CCD", "DDDD", "DDDDD", "ACC", "ABB", "AAAD", "AAAC", "DDD"],
             ["A1", "A2", "A3", "B1", "B5", "A5", "D1", "E", "C7", "B3", "A4", "B7", "D3"],
             "TRIUMPHANT",
             "C1",
             "ARCHANGELS WILL",
             "C5",
             [98,88,51,77,54,54,10,1,61,61,77,77,35,60,24])

Tactician = Hero("Tactician",
                ["A", "A", "A", "B", "B", "C"],
                ["SABER STRIKE 1", "SABER STRIKE 2", "SABER STRIKE 3", "CARPET BOMB", "STRATEGIC APPROACH",
                 "PROFITEER", "MANEUVER", "HIGHER GROUND!", "INDIRECT APPROACH", "INTERDICTION", "STRATEGIZE",
                 "RECONNASISSANCE"],
                ["AAA", "AAAA", "AAAAA", "AACC", "AAACC", "ABBB", "CCCC", "CCCCC", "AAAB", "ABBC", "BBBB", "CCC"],
                ["A1", "A2", "A3", "B1", "B5", "A5", "D1", "E", "B7", "C7", "B3", "D3"],
                "FLANK",
                "C1",
                "EXPLOIT",
                "C5",
                [98,88,51,50,31,72,10,1,92,72,54,35,60,24])

VampireLord = Hero("Vampire Lord",
                  ["A", "A", "A", "B", "B", "C"],
                  ["GOUGE 1", "GOUGE 2", "GOUGE 3", "GLAMOUR", "REND", "SANGUIMANCY", "BLOOD THIRST", "EXSANGUINATE!",
                   "HEMORRHAGE", "GASH", "PRESENCE", "INFLUENCE", "DRAW BLOOD"],
                  ["AAA", "AAAA", "AAAAA", "BBB", "AAACC", "BBBC", "CCCC", "CCCCC", "CCC", "AACC", "ABCC", "ABB",
                   "AAC"],
                  ["A1", "A2", "A3", "B1", "B5", "A5", "D1", "E", "D3", "C7", "A7", "B3", "C3"],
                  "POSSESS",
                  "C1",
                  "BLOOD MAGIC",
                  "C5",
                  [98,88,51,84,31,60,10,1,35,50,49,94,98,60,24])

Barbarian = Hero("Barbarian",
                ["A", "A", "A", "B", "B", "C"],
                ["SMACK 1", "SMACK 2", "SMACK 3", "STURDY BLOW", "OVERPOWER", "FORTITUDE 1", "FORTITUDE 2",
                 "FORTITUDE 3", "CRIT BASH", "RAGE!", "CRIT SMASH", "WAR CRY"],
                ["AAA", "AAAA", "AAAAA", "AACC", "AAACC", "BBB", "BBBB", "BBBBB", "CCCC", "CCCCC", "CCC", "AABB"],
                ["A1", "A2", "A3", "B1", "B5", "A5", "A6", "A7", "D1", "E", "D3", "B7"],
                "MIGHTY BLOW",
                "C1",
                "RECKLESS",
                "C5",
                [98,88,51,49,31,84,54,17,10,1,35,87,60,24])

Gunslinger = Hero("Gunslinger",
                 ["A", "A", "A", "B", "B", "C"],
                 ["REVOLVER 1", "REVOLVER 2", "REVOLVER 3", "BOUNTY HUNTER", "TAKE COVER", "DEADEYE",
                  "FILLEM WITH LEAD!", "PISTOL WHIP", "MARK THE TARGET", "ENFORCER"],
                 ["AAA", "AAAA", "AAAAA", "AACC", "AABBB", "CCCC", "CCCCC", "BBC", "BBB", "CCC"],
                 ["A1", "A2", "A3", "B1", "B5", "D1", "E", "C7", "B7", "D3"],
                 "SHOWDOWN",
                 "C1",
                 "FAN THE HAMMER",
                 "C5",
                 [98,88,51,49,49,10,1,82,84,35,60,24])

MoonElf = Hero("Moon Elf",
              ["A", "A", "A", "B", "B", "C"],
              ["LONGBOW 1", "LONGBOW 2", "LONGBOW 3", "DEMISING SHOT", "EXPLODING ARROW", "COVERED SHOT", "ECLIPSE",
               "LUNAR ECLIPSE!", "FOCUS", "EVASIVE ACTION", "LUNAR BLESSING", "MOON SHADOW"],
              ["AAA", "AAAA", "AAAAA", "AAACC", "ACCC", "AABBB", "CCCC", "CCCCC", "AAC", "BBB", "ABBC", "CCC"],
              ["A1", "A2", "A3", "B1", "B5", "A5", "D1", "E", "B3", "A7", "C7", "D3"],
              "ENTANGLING SHOT",
              "C1",
              "BLINDING SHOT",
              "C5",
              [98,88,51,31,24,49,10,1,86,84,72,35,60,24])

Ninja = Hero("Ninja",
            ["A", "A", "A", "B", "B", "C"],
            ["SLASH 1", "SLASH 2", "SLASH 3", "WALK THE LINE", "SMOKE SCREEN", "DEATH BLOSSOM", "SHADEWALK",
             "ASSASSINATE!", "KUJI-KIRI", "TIP OF THE BLADE", "MISDIRECT", "JUGULATE"],
            ["AAA", "AAAA", "AAAAA", "BBBB", "ABBC", "AAABB", "CCCC", "CCCCC", "BBBCC", "BBB", "AACC", "CCC"],
            ["A1", "A2", "A3", "B1", "B5", "A5", "D1", "E", "B7", "B3", "C7", "D3"],
            "POISON BLADE",
            "C1",
            "SHADOW FANG",
            "C5",
            [98,88,51,54,71,63,10,1,21,84,50,35,60,24])

Pyromancer = Hero("Pyromancer",
                 ["A", "A", "A", "B", "C", "D"],
                 ["FIREBALL 1", "FIREBALL 2", "FIREBALL 3", "BURNING SOUL 1", "BURNING SOUL 2", "BURNING SOUL 3",
                  "BURNING SOUL 4", "COMBUSTION", "PYROBLAST", "METEORITE",
                  "SCORCH THE EARTH!", "SCORCH", "BLAZING SOUL", "METEOROID"],
                 ["AAA", "AAAA", "AAAAA", "CC", "CCC", "CCCC", "CCCCC", "ABCD", "AAAD", "DDDD", "DDDDD", "AABB", "BBCC", "DDD"],
                 ["A1", "A2", "A3", "B1", "B2", "B3", "B4", "B5", "A5", "D1", "E", "C3", "C7", "D3"],
                 "HOT STREAK",
                 "C1",
                 "IGNITE",
                 "C5",
                 [98,88,51,70,36,10,1,54,77,10,1,49,28,35,60,24])

ShadowThief = Hero("Shadow Thief",
                  ["A", "A", "B", "B", "C", "D"],
                  ["DAGGER STRIKE 1", "DAGGER STRIKE 2", "DAGGER STRIKE 3", "PICKPOCKET 1", "PICKPOCKET 2",
                   "PICKPOCKET 3", "PICKPOCKET 4", "SHADOW DANCE", "CARDUCOPIA 1", "CARDUCOPIA 2", "CARDUCOPIA 3",
                   "CARDUCOPIA 4", "SHADOW STRIKE", "SHANK ATTACK", "SHADOW SHANK!"],
                  ["AAA", "AAAA", "AAAAA", "BB", "BBB", "BBBB", "BBBBB", "DDD", "CC", "CCC", "CCCC", "CCCCC",
                   "AADD", "ABCD", "DDDDD"],
                  ["A1", "A2", "A3", "A5", "A6", "A7", "A8", "C1", "C5", "C6", "C7", "C8", "B3", "B7", "E"],
                  "SHIFTY STRIKE",
                  "B1",
                  "INSIDIOUS STRIKE",
                  "B5",
                  [84,54,17,97,84,54,17,36,70,35,10,1,46,63,1,60,24])

Treant = Hero("Treant",
             ["A", "A", "A", "B", "B", "C"],
             ["SPLINTER 1", "SPLINTER 2", "SPLINTER 3", "TEND", "OVERGROWTH", "CULTIVATE", "PLANT", "BITTERROOT",
              "ENRAPTURE", "NATURES GRASP", "NATURES BLESSING", "WAKE THE FOREST!"],
             ["AAA", "AAAA", "AAAAA", "BBCC", "AABBB", "AACC", "ABB", "BBB", "AACC", "CCCC", "CCC", "CCCCC"],
             ["A1", "A2", "A3", "B1", "B5", "B3", "B7", "C3", "C7", "D1", "D3", "E"],
             "VENGEFUL VINES",
             "C1",
             "CALL OF THE WILD",
             "C5",
             [98,88,51,45,49,49,94,84,50,10,35,1,60,24])

CaptainMarvel = Hero("Captain Marvel",
             ["A","A","A","B","B","C"],
             ["PHOTON PUNCH 1","PHOTON PUNCH 2","PHOTON PUNCH 3","INNER LIGHT","AURORA WAVE","BARREL ROLL","SUPERNOVA",
              "BINARY BLAST!","COSMIC RECHARGE","BINARY RECHARGE","SKYWARD","INTERSTELLAR"],
             ["AAA","AAAA","AAAAA","AACC","AABCC","ABBB","CCCC","CCCCC","BBBC","BCC","AABB","CCC"],
             ["A1","A2","A3","B1","B5","A5","D1","E","B3","B7","C7","D3"],
             "PHOTONIC",
             "C1",
             "LIGHT TORPEDO",
             "C5",
             [98,88,51,49,31,72,10,1,60,60,87,35,60,24])

BlackPanther = Hero("Black Panther",
             ["A","A","A","B","B","C"],
             ["VIBRANIUM CLAWS 1","VIBRANIUM CLAWS 2","VIBRANIUM CLAWS 3","STEALTH STRIKE","HEART-SHAPED HERB",
              "TRIPLE THREAT","ANCESTRAL POWER","WAKANDA FOREVER!","PANTHER REFLEXES"],
             ["AAA","AAAA","AAAAA","AACC","BBB","AABBC","CCCC","CCCCC","CCC"],
             ["A1","A2","A3","B1","B5","C5","D1","E","D3"],
             "METEORIC PUNCH (small)",
             "C1",
             "METEORIC PUNCH (large)",
             "C3",
             [98,88,51,49,84,51,10,1,35,60,24])

Thor = Hero("Thor",
             ["A","A","A","B","B","C"],
             ["HAMMERED 1","HAMMERED 2","HAMMERED 3","MIGHTY SUMMON","BOOM BOOM!","ODINFORCE","CHAIN LIGHTNING",
              "ASGARDIAN BRAWN","BOTTLED LIGHTNING","RICOCHET!","FOR ASGARD!"],
             ["AAA","AAAA","AAAAA","ABBC","AACC","AABBB","AAACC","BBB","CCCC","CCC","CCCCC"],
             ["A1","A2","A3","B1","B3","A5","B5","C7","D1","D3","E"],
             "LIGHTNING ROD",
             "C1",
             "THUNDER BOLT",
             "C5",
             [98,88,51,72,49,48,31,84,10,35,1,60,24])

BlackWidow = Hero("Black Widow",
             ["A","A","B","B","B","C"],
             ["BATON STRIKE 1","BATON STRIKE 2","BATON STRIKE 3","INFILTRATE","SPY GAME","WIDOWS GAUNTLETS",
              "COVERT MISSION","SUBVERT","GRAPPLE","RECON","WIDOWS BITE!"],
             ["BBB","BBBB","BBBBB","AABC","AABCC","AAABB","AABB","ABBB","CCCC","CCC","CCCCC"],
             ["A1","A2","A3","B1","B3","B5","B7","C7","D1","D3","E"],
             "HACKED",
             "C1",
             "VENGEANCE",
             "C5",
             [98,88,51,72,27,48,87,92,10,35,1,60,24])

DrStrange = Hero("Dr Strange",
             ["A","A","B","B","C","D"],
             ["AUGURY","UNIVERSAL AWARENESS","ASTRAL INCURSION","BINDING","WAND OF WATOOMB","ALL-SEEING EYE!"],
             ["BBDD","BBBB","ABCD","AACC","DDDD","DDDDD"],
             ["A1","B1","B5","C5","D1","E"],
             "MYSTIC BONDS (small)",
             "C1",
             "MYSTIC BONDS (large)",
             "C3",
             [46,54,62,46,10,1,60,24])

Loki = Hero("Loki",
             ["A","A","B","B","C","D"],
             ["MOCKERY 1","MOCKERY 2","MOCKERY 3","CONFUDDLE","ANTICS 1","VILIFY","DOUBLE TAKE","GLORIOUS PURPOSE!","BEWILDER","LIARS GIFTS","ANTICS 2","MADE YOU LOOK"],
             ["AAA","AAAA","AAAAA","AABB","CC","ABCD","DDDD","DDDDD","BBC","BBB","CCC","DDD"],
             ["A1","A2","A3","A5","B1","B5","D1","E","C7","A7","B3","D3"],
             "VEX",
             "C1",
             "KNEEL TO ME",
             "C5",
             [84,54,17,78,70,62,10,1,82,84,35,35,60,24])

ScarletWitch = Hero("Scarlet Witch",
             ["A","A","A","B","C","D"],
             ["HEX BOLT 1","HEX BOLT 2","HEX BOLT 3","JINX","DARKHOLD","MIND BLAST","BEWITCH","PURE CHAOS","SEALED YOUR FATE!","CHAOS","ENERGY DRAIN","CONJURATION"],
             ["AAA","AAAA","AAAAA","AABB","CC","ABCD","ADDD","DDDD","DDDDD","DDD","BBC","AAC"],
             ["A1","A2","A3","A5","B1","B5","C5","D1","E","D3","C7","A4"],
             "SPELLCRAFT (small)",
             "C1",
             "SPELLCRAFT (large)",
             "C3",
             [98,88,51,50,70,53,24,10,1,35,54,86,60,24])

SpiderMan = Hero("Spider-Man",
             ["A","A","A","B","B","C"],
             ["PUNCH 1","PUNCH 2","PUNCH 3","WALL CRAWLER","C-C-C-COMBO","SPIDER-REFLEXES","VENOM PUNCH","VENOM SHOCKWAVE!","COMBO UP","WEB SHOT"],
             ["AAA","AAAA","AAAAA","AABBB","AACC","ABBC","CCCC","CCCCC","CCC","BBC"],
             ["A1","A2","A3","A5","B1","B5","D1","E","D3","B3"],
             "ENSNARE (small)",
             "C1",
             "ENSNARE (large)",
             "C3",
             [98,88,51,48,49,72,10,1,35,82,60,24])

hero = Barbarian


def setHero(hero):
   hero.set_as_active()

defcalc = False

def find_keepers(dice, objective):
   global defcalc
   obj = objective.copy()
   keepers = []
   rerollers = []
   success = False
   tmp_list = []
   exclude = []
   for i in range(len(dice)):
       if i == dice.index(dice[i]):
           tmp_list.append(dice[i])
       else:
           exclude.append(dice[i])
   tmp_list.extend(exclude)
   dice = tmp_list
   symdice = []
   for num in dice:
       sym = mydiceanatomy[num - 1]
       symdice.append(sym)
       if sym in obj:
           obj.remove(sym)
           keepers.append(num)
       else:
           rerollers.append(num)
   if not obj:
       success = True
   elif defcalc:
       pass
   else:
       freesix = 0
       if sixit:
           sixcheckobj = obj.copy()
           if mydiceanatomy[-1] in sixcheckobj:
               sixcheckobj.remove(mydiceanatomy[-1])
               freesix = 1
               if not sixcheckobj:
                   success = True
       if len(objective)-len(keepers)-freesix <= wilds:
           success = True
       if samesies:
           samesiesavailable = True
           keepersymcheck = []
           for num in keepers:
               sym = mydiceanatomy[num - 1]
               keepersymcheck.append(sym)
           samecheckobj = obj.copy()
           fullobjcheck = objective.copy()
           samecheckobjnodupes = list(set(fullobjcheck))
           wildcount = wilds
           if freesix == 1:
               keepersymcheck.append(mydiceanatomy[-1])
           for symb in samecheckobjnodupes:
               if wildcount > 0 and symb not in keepersymcheck:
                   keepersymcheck.append(symb)
                   wildcount -= 1
           for symb in samecheckobjnodupes:
               if keepersymcheck.count(symb) < fullobjcheck.count(symb) and samesiesavailable:
                   keepersymcheck.append(symb)
                   samesiesavailable = False
           if len(objective)-len(keepersymcheck) <= wildcount:
               success = True
   return keepers, rerollers, success


def find_number_keepers(dice, objective):
   obj = objective.copy()
   keepers = []
   rerollers = []
   success = False
   for num in dice:
       if num in obj:
           obj.remove(num)
           keepers.append(num)
       else:
           rerollers.append(num)
   if not obj:
       success = True
   return keepers, rerollers, success


def roll_dice(dice):
   for i in range(len(dice)):
       dice[i] = random.randint(1, 6)
   return dice


def go_for_obj(dice, objective, atmp, reroll):
   keepers, rerollers, success = find_keepers(dice, objective)
   if success:
       return success, dice
   for i in range(atmp):
       temp = roll_dice(rerollers)
       keepers = keepers + temp
       keepers, rerollers, success = find_keepers(keepers, objective)
       if success:
           break
   for i in range(reroll):
       temp2 = random.randint(1, 6)
       rerollers.append(temp2)
       del rerollers[0]
       keepers = keepers + rerollers
       keepers, rerollers, success = find_keepers(keepers, objective)
       if success:
           break
   result_dice = keepers + rerollers
   return success, result_dice


def count_consec(templist):
   templist = sorted(templist)
   lst = []
   [lst.append(x) for x in templist if x not in lst]
   consec = [1]
   for x, y in zip(lst, lst[1:]):
       if x == y - 1:
           consec[-1] += 1
       else:
           consec.append(1)
   return max(consec)


def is_straight(dice):
   global smst
   global lgst
   result = ""
   tf = False
   varient = [2, 3, 4, 5]
   for i in smst:
       if all(elem in dice for elem in i):
           result = "small"
           tf = True
           varient = i
           break
   for i in lgst:
       if all(elem in dice for elem in i):
           result = "large"
           tf = True
           varient = i
           break
   return tf, result, varient


def is_straight_with_wilds(dice):
   global smst
   global lgst
   global wilds
   result = ""
   tf = False
   if wilds > 0 or sixit:
       dieset = set(dice)
       remove = False
       for u in smst:
           matches = 0
           smstset = set(u)
           if sixit and 6 not in dieset and 6 in smstset:
               dieset.add(6)
               remove = True
           matches = len(dieset & smstset)
           if remove:
               dieset.remove(6)
               remove = False
           if matches + wilds >= 4:
               result = "small"
               tf = True
               break
       for o in lgst:
           matches = 0
           lgstset = set(o)
           if sixit and 6 not in dieset and 6 in lgstset:
               dieset.add(6)
               remove = True
           matches = len(dieset & lgstset)
           if remove:
               dieset.remove(6)
               remove = False
           if matches + wilds >= 5:
               result = "large"
               tf = True
               break
   return tf, result


def go_for_sts(dice, atmp, reroll, smstop):
   keep = dice
   stf1, smlg1, var1 = is_straight(keep)
   keep1, toss1, success1 = find_number_keepers(keep, var1)
   if success1 and smstop:
       return keep1, stf1, smlg1
   for i in range(atmp):
       stf, smlg, var = is_straight(keep)
       keep, toss, success = find_number_keepers(keep, var)
       toss = roll_dice(toss)
       keep = keep + toss
       if success and smstop:
           break
   for i in range(reroll):
       stf, smlg, var = is_straight(keep)
       keep, toss, success = find_number_keepers(keep, var)
       temp = random.randint(1, 6)
       toss.append(temp)
       del toss[0]
       keep = keep + toss
       if success and smstop:
           break
       keep = keep + toss
       if success and smstop:
           break
   stf, smlg, var = is_straight(keep)
   return keep, stf, smlg


def name_from_address(address):
   result = abilitynames[0]
   if address in abilitylocations:
       locationindex = abilitylocations.index(address)
       result = abilitynames[locationindex]
   elif address == smalllocation:
       result = smallname
   elif address == largelocation:
       result = largename
   return result


def rollobj_from_address(address):
   result = rollobjs[0]
   if address in abilitylocations:
       locationindex = abilitylocations.index(address)
       result = rollobjs[locationindex]
   elif address == smalllocation:
       result = smst
   elif address == largelocation:
       result = lgst
   return result


def find_default_odds():
   global defcalc
   global simcount
   prevsimcount = simcount
   simcount = 10000
   defcalc = True
   progvalincrement = simcount / ((len(abilitynames) * simcount) + simcount)
   progval = 0
   global dice
   dice = []
   for i in range(5):
       dice.append(int(curentdicevalues[i]))
   defoddssimresdect = {}
   for name, ability in zip(abilitynames, rollobjs):
       count = 0
       for q in range(simcount):
           progval += progvalincrement
           if q % 100 == 0:
               progress_bar.update(current_count=progval)
           tempdice = roll_dice(dice)
           a, b = go_for_obj(tempdice, ability, 2, 0)
           if a:
               count += 1
       odds = round(100 * count / simcount)
       defoddssimresdect[name] = odds
   smcnt = 0
   lgcnt = 0
   for z in range(simcount):
       progval += progvalincrement
       if z % 10 == 0:
           progress_bar.update(current_count=progval)
       tempdice = roll_dice(dice)
       res, stf, smlg = go_for_sts(tempdice, 2, 0, False)
       if stf:
           if smlg == "large":
               smcnt += 1
               lgcnt += 1
           elif smlg == "small":
               smcnt += 1
   smodds = round(100 * smcnt / simcount)
   lgodds = round(100 * lgcnt / simcount)
   defoddssimresdect[smallname] = smodds
   defoddssimresdect[largename] = lgodds
   defcalc = False
   simcount = prevsimcount
   return defoddssimresdect

def run_norm_sim():
   progvalincrement = simcount/((len(abilitynames)*simcount)+simcount)
   progval = 0
   global dice
   dice = []
   for i in range(5):
       dice.append(int(curentdicevalues[i]))
   normsimresdect = {}
   for name, ability in zip(abilitynames, rollobjs):
       count = 0
       for q in range(simcount):
           progval += progvalincrement
           if q % 100 == 0:
               progress_bar.update(current_count=progval)
           a, b = go_for_obj(dice, ability, rollatmps, rerolls)
           if a:
               count += 1
       odds = round(100 * count / simcount)
       normsimresdect[name] = odds
   smcnt = 0
   lgcnt = 0
   for z in range(simcount):
       progval += progvalincrement
       if z % 10 == 0:
           progress_bar.update(current_count=progval)
       res, stf, smlg = go_for_sts(dice, rollatmps, rerolls, False)
       wstf, wssmlg = is_straight_with_wilds(res)
       if wstf:
           stf = True
           smlg = wssmlg
       if stf:
           if smlg == "large":
               smcnt += 1
               lgcnt += 1
           elif smlg == "small":
               smcnt += 1
   smodds = round(100 * smcnt / simcount)
   lgodds = round(100 * lgcnt / simcount)
   normsimresdect[smallname] = smodds
   normsimresdect[largename] = lgodds
   return normsimresdect

def run_norm_sim_exe(default, dices, rollatmps, rerolls):
    anatomy = ""
    for s in mydiceanatomy:
        anatomy+=s
    abilities_to_calc = ""
    for s in rawrollobjs:
        if abilities_to_calc!= "":
            abilities_to_calc+=";"
        abilities_to_calc+=s
    for s in ["SMALL", "BIG"]:
        if abilities_to_calc!= "":
            abilities_to_calc+=";"
        abilities_to_calc+=s
    data = ['DiceThroneSimulator.exe',"-default", default, "-hero",currenthero, "-dice", dices, "-rolls", str(rollatmps), "-rerolls", str(rerolls) ]
    data.append("-sixit")
    data.append(window['-SIXIT-'].get())
    data.append("-samesies")
    data.append(window['-SAMESIES-'].get())
    data.append("-tipit")
    data.append(window['-TIPIT-'].get())
    data.append("-wild")
    data.append(window['-WILD-'].get())
    data.append("-2wild")
    data.append(window['-2WILD-'].get())
    data.append("-swild")
    data.append(window['-SWILD-'].get())
    data.append("-cheer")
    data.append(window['-CHEER-'].get())
    data.append("-probabilitymanipulation")
    data.append(window['-PROBMANI-'].get())
    data.append("-cp")
    data.append(window['-CP-'].get())
    data.append("-cards")
    data.append(window['-CARDS-'].get())
    data.append("-abilities")
    data.append(abilities_to_calc)
    data.append("-anatomy")
    data.append(anatomy)
    scarlett = window['-SCARLETT-'].get()
    if scarlett:
        data.append("-scarlett")
        data.append("true")
    
    print(data)
    result = subprocess.run(data, stdout=subprocess.PIPE)
    output = result.stdout.decode('utf-8')
    print(output)
    return output

def run_fallback_sim_exe(prime_obj, default, dices, rollatmps, rerolls):
    anatomy = ""
    for s in mydiceanatomy:
        anatomy+=s
    abilities_to_calc = ""
    chased_abil = ""
    for s in prime_obj:
        chased_abil+=s
    for s in rawrollobjs:
        if abilities_to_calc!= "":
            abilities_to_calc+=";"
        abilities_to_calc+=s
    for s in ["SMALL", "BIG"]:
        if abilities_to_calc!= "":
            abilities_to_calc+=";"
        abilities_to_calc+=s
    data = ['DiceThroneSimulator.exe',"-default", default, "-hero",currenthero, "-dice", dices, "-rolls", str(rollatmps), "-rerolls", str(rerolls) ]
    data.append("-sixit")
    data.append(window['-SIXIT-'].get())
    data.append("-samesies")
    data.append(window['-SAMESIES-'].get())
    data.append("-tipit")
    data.append(window['-TIPIT-'].get())
    data.append("-wild")
    data.append(window['-WILD-'].get())
    data.append("-2wild")
    data.append(window['-2WILD-'].get())
    data.append("-swild")
    data.append(window['-SWILD-'].get())
    data.append("-cheer")
    data.append(window['-CHEER-'].get())
    data.append("-probabilitymanipulation")
    data.append(window['-PROBMANI-'].get())
    data.append("-cp")
    data.append(window['-CP-'].get())
    data.append("-cards")
    data.append(window['-CARDS-'].get())
    data.append("-abilities")
    data.append(abilities_to_calc)
    data.append("-anatomy")
    data.append(anatomy)
    data.append("-chase")
    data.append(chased_abil)
    print(data)
    result = subprocess.run(data, stdout=subprocess.PIPE)
    output = result.stdout.decode('utf-8')
    print(output)
    return output
    
def run_fallback_sim(prime_obj):
   global dice
   dice = []
   for i in range(5):
       dice.append(int(curentdicevalues[i]))
   abilitycounterdict = {}
   for abilityname in abilitynames:
       abilitycounterdict[abilityname] = 0
   fallbacksimdict = {}
   progvalincrement = 1
   progval = 0
   smcnt = 0
   lgcnt = 0
   goingforstraights = False
   smststop = False

   if prime_obj == smst:
       goingforstraights = True
       smststop = True
   elif prime_obj == lgst:
       goingforstraights = True

   if not goingforstraights:
       for q in range(simcount):
           progval += progvalincrement
           if q % 100 == 0:
               progress_bar.update(current_count=progval)
           s, rd = go_for_obj(dice, prime_obj, rollatmps, rerolls)
           for name, ability in zip(abilitynames, rollobjs):
               suc, temp = go_for_obj(rd, ability, 0, 0)
               if suc:
                   abilitycounterdict[name] += 1
           temp1, stf, smlg = go_for_sts(rd, 0, 0, smststop)
           wstf, wssmlg = is_straight_with_wilds(rd)
           if wstf:
               stf = True
               smlg = wssmlg
           if stf:
               if smlg == "large":
                   smcnt += 1
                   lgcnt += 1
               elif smlg == "small":
                   smcnt += 1
   else:
       for z in range(simcount):
           progval += progvalincrement
           if z % 100 == 0:
               progress_bar.update(current_count=progval)
           rd, stf, smlg = go_for_sts(dice, rollatmps, rerolls, smststop)
           wstf, wssmlg = is_straight_with_wilds(rd)
           if wstf:
               stf = True
               smlg = wssmlg
           if stf:
               if smlg == "large":
                   smcnt += 1
                   lgcnt += 1
               elif smlg == "small":
                   smcnt += 1
           for name, ability in zip(abilitynames, rollobjs):
               suc, temp = go_for_obj(rd, ability, 0, 0)
               if suc:
                   abilitycounterdict[name] += 1

   smodds = round(100 * smcnt / simcount)
   lgodds = round(100 * lgcnt / simcount)
   for key in abilitycounterdict:
       odds = round(100 * abilitycounterdict[key] / simcount)
       fallbacksimdict[key] = odds
   fallbacksimdict[smallname] = smodds
   fallbacksimdict[largename] = lgodds
   return fallbacksimdict


def print_results(resdict):
   for abilname in resdict:
       grid = alllocations[allabilitynames.index(abilname)]
       namekey = '-'+grid+'NAME-'
       window1[namekey].update(abilname)
       abilodds = str(resdict[abilname])+'%'
       oddskey = '-'+grid+'%-'
       buttonkey = '-'+grid+'CHASE-'
       window1[oddskey].update(abilodds)
       window1[buttonkey].update(visible=True)
       window1[oddskey].update(visible=True)
       window1[namekey].update(visible=True)
       for col in ['A','B','C','D']:
           for row in ['1','2','3','4','5','6','7','8']:
               adrs = col+row
               if adrs not in alllocations:
                   hidenamekey = '-'+adrs+'NAME-'
                   hideoddskey = '-'+adrs+'%-'
                   hidebuttonkey = '-'+adrs+'CHASE-'
                   window1[hidenamekey].update(visible=False)
                   window1[hideoddskey].update(visible=False)
                   window1[hidebuttonkey].update(visible=False)
   return

def print_results2(returnstring, dice):
   startparse = False
   resdict = {}
   #print("pars return")
   #print(returnstring)
   #print(rawrollobjs)
   lines = returnstring.split("\r\n")
   #print(allabilitynames)
   for x in lines:
        if x == "RESULTS":
            startparse = True
            continue
        if startparse == False:
            continue
        if x == "":
            continue
        data = x.split(" ")
        rollobj = data[0]
        odd = data[1]
        reroll = data[2]
        if rollobj in ["SMALL", "BIG"]:
            if rollobj == "SMALL":
                abiname = allabilitynames[-2]
            else:
                abiname = allabilitynames[-1]
        else:
            abiname = allabilitynames[rawrollobjs.index(rollobj)]
        #print(abiname, odd)
        if dice == "":
            resdict[abiname] = odd + "%"
        else:
            reroll_str = ""
            for i in [0,1,2,3,4]:
                if(reroll[i] == '1'):
                    reroll_str += dice[i]
            if reroll_str == "":
                resdict[abiname] = odd + "%"
            else:
                resdict[abiname] = odd + "% (" + reroll_str + ")"
   #print(resdict)
   for abilname in resdict:
       grid = alllocations[allabilitynames.index(abilname)]
       namekey = '-'+grid+'NAME-'
       window1[namekey].update(abilname)
       abilodds = str(resdict[abilname])
       oddskey = '-'+grid+'%-'
       buttonkey = '-'+grid+'CHASE-'
       window1[oddskey].update(abilodds)
       window1[buttonkey].update(visible=True)
       window1[oddskey].update(visible=True)
       window1[namekey].update(visible=True)
       for col in ['A','B','C','D']:
           for row in ['1','2','3','4','5','6','7','8']:
               adrs = col+row
               if adrs not in alllocations:
                   hidenamekey = '-'+adrs+'NAME-'
                   hideoddskey = '-'+adrs+'%-'
                   hidebuttonkey = '-'+adrs+'CHASE-'
                   window1[hidenamekey].update(visible=False)
                   window1[hideoddskey].update(visible=False)
                   window1[hidebuttonkey].update(visible=False)
   return

def getCardsString():
    retval = ""
    strings= ["SIXIT", "SAMESIES", "TIPIT", "WILD", "2WILD", "SWILD"]
    strings2= ["Sixit", "Samesies", "Tipit", "Wild", "Twice as Wild", "Slightly Wild"]
    level = ["", "I", "II", "III", "IV", "V"]
    for index in [0,1,2,3,4,5]:
        val = int(window['-'+strings[index]+'-'].get())
        if(val>0):
            if retval != "":
                retval + " | "
            retval += strings2[index] + " "+ level[val]
    if retval == "":
        return ""
    retval += " | max CP to spend: " + window['-CP-'].get()
    retval += " | max Cards to use: " + window['-CARDS-'].get()
    return retval

menu_def_med = [['Hero',
                ['Season 1',
                   ['Barbarian','Moon Elf','Pyromancer','Shadow Thief','Monk','Paladin','Ninja','Treant',],
                'Season 2',
                   ['Gunslinger','Samurai','Tactician','Huntress','Cursed Pirate','Artificer','Seraph','Vampire Lord',],
                'Marvel',
                   ['Thor','Loki','Scarlet Witch','Spider-Man','Captain Marvel','Black Panther','Dr Strange','Black Widow',],]],
               #['Settings',
               # ['Simulation Accuracy',
               #    ['Low (Fast)', 'Medium (Medium)', 'High (Slow)','---------------','Default: Medium',],]],
               ['Homebrew',
                ['Create Homebrew','Load Homebrew','Edit Current Hero',],],
               ['Help',
                ['About...', 'Chase Definition', 'How to Use', 'Other Notes',],]]

menuelem = sg.Menu(menu_def_med)



def place(elem1, elem2):
    '''
    Places element provided into a Column element so that its placement in the layout is retained.
    :param elem: the element to put into the layout
    :return: A column element containing the provided element
    '''
    return sg.Column([[elem1, elem2]], pad=(0,0))


def make_win1():
   sg.theme('DarkBlue1')
   rerollvalues = []
   for num in range(100):
       rerollvalues.append(str(num))
   rollattemptvalues = []
   for num in range(100):
       rollattemptvalues.append(str(num))

   constantrows = [[sg.Text(currenthero, font=("Helvetica", fontsize+10), k='-HERONAME-')],
                   [sg.Text('Roll Attempts:', font=("Helvetica", fontsize), k='-RAtxt-'),
                       place(sg.Spin(values=(rollattemptvalues), initial_value=2, font=("Helvetica", fontsize), size=(2,1),enable_events=True, k='-RA-'),sg.Spin(values=(rollattemptvalues), initial_value=2, font=("Helvetica", fontsize), text_color = "red", size=(2,1),enable_events=True, k='-RARed-', visible = False)),
                       sg.Text('', size=(5,1)),
                       sg.Text('Re-Rolls:', font=("Helvetica", fontsize)),
                       sg.Spin(values=(rerollvalues), font=("Helvetica", fontsize), size=(2,1), k='-RR-'),
                       sg.Text('', size=(5, 1)),
                       #sg.Checkbox('Six-It', default=False, font=("Helvetica", fontsize), enable_events=True, key='-SIXIT-'),
                       sg.Text(' CP (0):', font=("Helvetica", fontsize), k='-MAXCPTEXT-'),
                       sg.Spin(values=('0','1', '2', '3', '4'), initial_value=0, font=("Helvetica", fontsize), size=(2, 1), k='-CP-'),
                       sg.Text(' Cards (0):', font=("Helvetica", fontsize), k="-MAXCARDSTEXT-"),
                       sg.Spin(values=('0','1', '2', '3', '4'), initial_value=0, font=("Helvetica", fontsize), size=(2, 1), k='-CARDS-'),
                    ],
                    [
                       sg.Text('Sixit-LvL:', font=("Helvetica", fontsize)),
                       sg.Spin(values=('0','1', '2', '3', '4'), font=("Helvetica", fontsize),enable_events=True, k='-SIXIT-'),
                       sg.Text('Samesies-LvL:', font=("Helvetica", fontsize)),
                       sg.Spin(values=('0','1', '2', '3', '4'), font=("Helvetica", fontsize),enable_events=True, k='-SAMESIES-'),
                       sg.Text('Tip It-LvL:', font=("Helvetica", fontsize)),
                       sg.Spin(values=('0','1', '2', '3', '4'), font=("Helvetica", fontsize),enable_events=True, k='-TIPIT-'),
                       sg.Text('Wild-LvL:', font=("Helvetica", fontsize)),
                       sg.Spin(values=('0','1', '2', '3', '4'), initial_value=0, font=("Helvetica", fontsize), size=(2, 1),enable_events=True, k='-WILD-'),
                       sg.Text(' Twice as Wild-LvL:', font=("Helvetica", fontsize)),
                       sg.Spin(values=('0','1', '2', '3', '4'), initial_value=0, font=("Helvetica", fontsize), size=(2, 1),enable_events=True, k='-2WILD-'),
                       sg.Text(' Slightly Wild-LvL:', font=("Helvetica", fontsize)),
                       sg.Spin(values=('0','1', '2', '3', '4'), initial_value=0, font=("Helvetica", fontsize), size=(2, 1),enable_events=True, k='-SWILD-'),
                    ],
                    [
                       sg.Text('(3x)Cheer:', font=("Helvetica", fontsize)),
                       sg.Spin(values=('0','1'), font=("Helvetica", fontsize),enable_events=True, k='-CHEER-'),
                       sg.Text('Probability Manipulation:', font=("Helvetica", fontsize)),
                       sg.Spin(values=('0','1', '2'), font=("Helvetica", fontsize),enable_events=True, k='-PROBMANI-'),
                    ],
                   [
                       sg.Text('Dice:', font=("Helvetica", fontsize)),
                       sg.Spin(values=('1', '2', '3', '4', '5', '6'),font=("Helvetica", fontsize), k='-D1-'),
                       sg.Spin(values=('1', '2', '3', '4', '5', '6'), font=("Helvetica", fontsize), k='-D2-'),
                       sg.Spin(values=('1', '2', '3', '4', '5', '6'), font=("Helvetica", fontsize), k='-D3-'),
                       sg.Spin(values=('1', '2', '3', '4', '5', '6'), font=("Helvetica", fontsize), k='-D4-'),
                       sg.Spin(values=('1', '2', '3', '4', '5', '6'), font=("Helvetica", fontsize), k='-D5-'),
                       sg.Checkbox('5th Die is Reality Warped', default=False, font=("Helvetica", fontsize), k='-SCARLETT-'),
                       ],
                   [sg.Text('Default Odds', font=("Helvetica", fontsize-5)),
                       sg.Button('Calculate', font=("Helvetica", fontsize-5), k='-DEFAULTODDS-'),
                       sg.Text('', size=(5, 1)),
                       sg.Text('Chase All', font=("Helvetica", fontsize-5)),
                       sg.Button('Calculate', font=("Helvetica", fontsize - 5), k='-NORMSIM-'),
                       sg.Text('', size=(5, 1)),
                       sg.Text('Chase', font=("Helvetica", fontsize - 5)),
                       sg.Text(name_from_address(chaseability), font=("Helvetica", fontsize - 10), k='-CHASE-'),
                       sg.Button('Calculate', font=("Helvetica", fontsize - 5), k='-CHASESIM-')
                       ],
                   [sg.Text('Displaying Results For:', font=("Helvetica", fontsize-6)),
                       sg.Text('Default Odds', font=("Helvetica", fontsize-6), key='-DISPLAYING-')],
                   [sg.Text('', font=("Helvetica", fontsize-6), key='-DISPLAYING2-')]
                   ]
   abilityfont = ("Helvetica", fontsize - 10)
   abiltiytextlength = (20,1)
   column1toplayout = [[sg.Button('Chase', k='-A1CHASE-'), sg.Text('10%', k='-A1%-'), sg.Text('NameA1', k='-A1NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-A2CHASE-'), sg.Text('10%', k='-A2%-'), sg.Text('NameA2', k='-A2NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-A3CHASE-'), sg.Text('10%', k='-A3%-'), sg.Text('NameA3', k='-A3NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-A4CHASE-'), sg.Text('10%', k='-A4%-'), sg.Text('NameA4', k='-A4NAME-', size=abiltiytextlength, font=abilityfont)]]
   column1top = [sg.Frame('',column1toplayout)]
   column1botlayout = [[sg.Button('Chase', k='-A5CHASE-'), sg.Text('10%', k='-A5%-'), sg.Text('NameA5', k='-A5NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-A6CHASE-'), sg.Text('10%', k='-A6%-'), sg.Text('NameA6', k='-A6NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-A7CHASE-'), sg.Text('10%', k='-A7%-'), sg.Text('NameA7', k='-A7NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-A8CHASE-'), sg.Text('10%', k='-A8%-'), sg.Text('NameA8', k='-A8NAME-', size=abiltiytextlength, font=abilityfont)]]
   column1bot = [sg.Frame('',column1botlayout)]
   column1 = sg.Column([column1top,column1bot])
   column1layout = [[sg.Frame('',[[column1]])]]

   column2toplayout = [[sg.Button('Chase', k='-B1CHASE-'), sg.Text('10%', k='-B1%-'), sg.Text('NameB1', k='-B1NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-B2CHASE-'), sg.Text('10%', k='-B2%-'), sg.Text('NameB2', k='-B2NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-B3CHASE-'), sg.Text('10%', k='-B3%-'), sg.Text('NameB3', k='-B3NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-B4CHASE-'), sg.Text('10%', k='-B4%-'), sg.Text('NameB4', k='-B4NAME-', size=abiltiytextlength, font=abilityfont)]]
   column2top = [sg.Frame('',column2toplayout)]
   column2botlayout = [[sg.Button('Chase', k='-B5CHASE-'), sg.Text('10%', k='-B5%-'), sg.Text('NameB5', k='-B5NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-B6CHASE-'), sg.Text('10%', k='-B6%-'), sg.Text('NameB6', k='-B6NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-B7CHASE-'), sg.Text('10%', k='-B7%-'), sg.Text('NameB7', k='-B7NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-B8CHASE-'), sg.Text('10%', k='-B8%-'), sg.Text('NameB8', k='-B8NAME-', size=abiltiytextlength, font=abilityfont)]]
   column2bot = [sg.Frame('',column2botlayout)]
   column2 = sg.Column([column2top,column2bot])
   column2layout = [[sg.Frame('',[[column2]])]]

   column3toplayout = [[sg.Button('Chase', k='-C1CHASE-'), sg.Text('10%', k='-C1%-'), sg.Text('NameC1', k='-C1NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-C2CHASE-'), sg.Text('10%', k='-C2%-'), sg.Text('NameC2', k='-C2NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-C3CHASE-'), sg.Text('10%', k='-C3%-'), sg.Text('NameC3', k='-C3NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-C4CHASE-'), sg.Text('10%', k='-C4%-'), sg.Text('NameC4', k='-C4NAME-', size=abiltiytextlength, font=abilityfont)]]
   column3top = [sg.Frame('',column3toplayout)]
   column3botlayout = [[sg.Button('Chase', k='-C5CHASE-'), sg.Text('10%', k='-C5%-'), sg.Text('NameC5', k='-C5NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-C6CHASE-'), sg.Text('10%', k='-C6%-'), sg.Text('NameC6', k='-C6NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-C7CHASE-'), sg.Text('10%', k='-C7%-'), sg.Text('NameC7', k='-C7NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-C8CHASE-'), sg.Text('10%', k='-C8%-'), sg.Text('NameC8', k='-C8NAME-', size=abiltiytextlength, font=abilityfont)]]
   column3bot = [sg.Frame('',column3botlayout)]
   column3 = sg.Column([column3top,column3bot])
   column3layout = [[sg.Frame('',[[column3]])]]

   column4toplayout = [[sg.Button('Chase', k='-D1CHASE-'), sg.Text('10%', k='-D1%-'), sg.Text('NameD1', k='-D1NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-D2CHASE-'), sg.Text('10%', k='-D2%-'), sg.Text('NameD2', k='-D2NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-D3CHASE-'), sg.Text('10%', k='-D3%-'), sg.Text('NameD3', k='-D3NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-D4CHASE-'), sg.Text('10%', k='-D4%-'), sg.Text('NameD4', k='-D4NAME-', size=abiltiytextlength, font=abilityfont)]]
   column4top = [sg.Frame('',column4toplayout)]
   column4botlayout = [[sg.Button('Chase', k='-D5CHASE-'), sg.Text('10%', k='-D5%-'), sg.Text('NameD5', k='-D5NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-D6CHASE-'), sg.Text('10%', k='-D6%-'), sg.Text('NameD6', k='-D6NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-D7CHASE-'), sg.Text('10%', k='-D7%-'), sg.Text('NameD7', k='-D7NAME-', size=abiltiytextlength, font=abilityfont)],
                       [sg.Button('Chase', k='-D8CHASE-'), sg.Text('10%', k='-D8%-'), sg.Text('NameD8', k='-D8NAME-', size=abiltiytextlength, font=abilityfont)]]
   column4bot = [sg.Frame('',column4botlayout)]
   column4 = sg.Column([column4top,column4bot])
   column4layout = [[sg.Frame('',[[column4]])]]

   layout = [[menuelem],
             [constantrows],
             [sg.Column(column1layout), sg.Column(column2layout), sg.Column(column3layout), sg.Column(column4layout)],
             [sg.Frame('',[[sg.Button('Chase', k='-ECHASE-'), sg.Text('10%', k='-E%-'), sg.Text('NameE', k='-ENAME-')]])]
             ]
   return sg.Window('Real Time DT Odds', layout, relative_location=(0,0),
                    finalize=True, resizable=True, element_justification='center')


def make_win2(location, otherwindowsize):
   layout = [[sg.Text('Running Simulation...')],
             [sg.ProgressBar(max_value=simcount, orientation='h', size=(20, 20), key='-PROGRESS BAR-')]]
   return sg.Window('Progress Bar', layout, location=(location[0]+(otherwindowsize[0]/2)-125,location[1]+\
                                                      (otherwindowsize[1]/2)-31), keep_on_top=True, finalize=True)


vislayout = 1
maxlayouts = 4


def make_win3():
   layout1 = [[sg.Text('Enter Hero Name', font=("Helvetica", fontsize + 10))],
              [sg.InputText('Name', size=(20,1), font=("Helvetica", fontsize + 10), k='-CUSTNAME-')],
              [sg.Text('')],
              [sg.Button('Exit'), sg.Button('Continue to Dice Anatomy')]
              ]

   layout2 = [[sg.Text('Set Dice Anatomy', font=("Helvetica", fontsize + 10))],
              [sg.Text('Numbers:', size=(9,1), font=("Helvetica", fontsize)),
               sg.Text('1', size=(3,1), font=("Helvetica", fontsize)),
               sg.Text('2', size=(3,1), font=("Helvetica", fontsize)),
               sg.Text('3', size=(3,1), font=("Helvetica", fontsize)),
               sg.Text('4', size=(3,1), font=("Helvetica", fontsize)),
               sg.Text('5', size=(3,1), font=("Helvetica", fontsize)),
               sg.Text('6', size=(3,1), font=("Helvetica", fontsize)),],
              [sg.Text('Symbols:', size=(8,1), font=("Helvetica", fontsize)),
               sg.Spin(values=('A', 'B', 'C', 'D', 'E', 'F'), readonly=True, text_color='black', size=(2,1), font=("Helvetica", fontsize), k='-CD1-'),
               sg.Spin(values=('A', 'B', 'C', 'D', 'E', 'F'), readonly=True, text_color='black', size=(2,1), font=("Helvetica", fontsize), k='-CD2-'),
               sg.Spin(values=('A', 'B', 'C', 'D', 'E', 'F'), readonly=True, text_color='black', size=(2,1), font=("Helvetica", fontsize), k='-CD3-'),
               sg.Spin(values=('A', 'B', 'C', 'D', 'E', 'F'), readonly=True, text_color='black', size=(2,1), font=("Helvetica", fontsize), k='-CD4-'),
               sg.Spin(values=('A', 'B', 'C', 'D', 'E', 'F'), readonly=True, text_color='black', size=(2,1), font=("Helvetica", fontsize), k='-CD5-'),
               sg.Spin(values=('A', 'B', 'C', 'D', 'E', 'F'), readonly=True, text_color='black', size=(2,1), font=("Helvetica", fontsize), k='-CD6-')],
              [sg.Text('')],
              [sg.Button('Go Back'), sg.Button('Continue to Straights')]
              ]
   pickerbuttonsize = (10,1)
   custcolumn1toplayout = [[sg.Button('A1', size=pickerbuttonsize)],
                           [sg.Button('A2', size=pickerbuttonsize)],
                           [sg.Button('A3', size=pickerbuttonsize)],
                           [sg.Button('A4', size=pickerbuttonsize)]]
   custcolumn1top = [sg.Frame('', custcolumn1toplayout)]
   custcolumn1botlayout = [[sg.Button('A5', size=pickerbuttonsize)],
                           [sg.Button('A6', size=pickerbuttonsize)],
                           [sg.Button('A7', size=pickerbuttonsize)],
                           [sg.Button('A8', size=pickerbuttonsize)]]
   custcolumn1bot = [sg.Frame('', custcolumn1botlayout)]
   custcolumn1 = sg.Column([custcolumn1top, custcolumn1bot])
   custcolumn1layout = [[sg.Frame('', [[custcolumn1]])]]

   custcolumn2toplayout = [[sg.Button('B1', size=pickerbuttonsize)],
                           [sg.Button('B2', size=pickerbuttonsize)],
                           [sg.Button('B3', size=pickerbuttonsize)],
                           [sg.Button('B4', size=pickerbuttonsize)]]
   custcolumn2top = [sg.Frame('', custcolumn2toplayout)]
   custcolumn2botlayout = [[sg.Button('B5', size=pickerbuttonsize)],
                           [sg.Button('B6', size=pickerbuttonsize)],
                           [sg.Button('B7', size=pickerbuttonsize)],
                           [sg.Button('B8', size=pickerbuttonsize)]]
   custcolumn2bot = [sg.Frame('', custcolumn2botlayout)]
   custcolumn2 = sg.Column([custcolumn2top, custcolumn2bot])
   custcolumn2layout = [[sg.Frame('', [[custcolumn2]])]]

   custcolumn3toplayout = [[sg.Button('C1', size=pickerbuttonsize)],
                           [sg.Button('C2', size=pickerbuttonsize)],
                           [sg.Button('C3', size=pickerbuttonsize)],
                           [sg.Button('C4', size=pickerbuttonsize)]]
   custcolumn3top = [sg.Frame('', custcolumn3toplayout)]
   custcolumn3botlayout = [[sg.Button('C5', size=pickerbuttonsize)],
                           [sg.Button('C6', size=pickerbuttonsize)],
                           [sg.Button('C7', size=pickerbuttonsize)],
                           [sg.Button('C8', size=pickerbuttonsize)]]
   custcolumn3bot = [sg.Frame('', custcolumn3botlayout)]
   custcolumn3 = sg.Column([custcolumn3top, custcolumn3bot])
   custcolumn3layout = [[sg.Frame('', [[custcolumn3]])]]

   custcolumn4toplayout = [[sg.Button('D1', size=pickerbuttonsize)],
                           [sg.Button('D2', size=pickerbuttonsize)],
                           [sg.Button('D3', size=pickerbuttonsize)],
                           [sg.Button('D4', size=pickerbuttonsize)]]
   custcolumn4top = [sg.Frame('', custcolumn4toplayout)]
   custcolumn4botlayout = [[sg.Button('D5', size=pickerbuttonsize)],
                           [sg.Button('D6', size=pickerbuttonsize)],
                           [sg.Button('D7', size=pickerbuttonsize)],
                           [sg.Button('D8', size=pickerbuttonsize)]]
   custcolumn4bot = [sg.Frame('', custcolumn4botlayout)]
   custcolumn4 = sg.Column([custcolumn4top, custcolumn4bot])
   custcolumn4layout = [[sg.Frame('', [[custcolumn4]])]]

   possitionpickerlayout = [sg.Column(custcolumn1layout), sg.Column(custcolumn2layout), sg.Column(custcolumn3layout),
              sg.Column(custcolumn4layout)]

   layout3 = [[sg.Text('Select Straight Type:', font=("Helvetica", fontsize)),
              sg.DropDown(['Small Straight', 'Large Straight'], default_value='Small Straight', size=(12,1), font=("Helvetica", fontsize), readonly=True, key='-STDROP-'),
              sg.Button('SELECT', size=(10,2), k='-SETTYPE-')],
              [sg.Text('Enter Small Straight Name', font=("Helvetica", fontsize + 10), k='-STNAMEPROMPT-')],
              [sg.InputText('Small Name', size=(20,1), font=("Helvetica", fontsize + 10), k='-STNAME-'), sg.Button('SET NAME', size=(10,2), k='-SETNAME-')],
              [sg.Text('')],
              [sg.Text('Select Small Straight Location', font=("Helvetica", fontsize + 10), k='-STLOCATIONPROMPT-')],
              possitionpickerlayout,
              [sg.Text('Small Straight', font=("Helvetica", fontsize), key='-SMLGDISP-'),
               sg.Text('Selected Position: C1', font=("Helvetica", fontsize), key='-SELECTEDPOS-')],
              [sg.Text('Small Straight', font=("Helvetica", fontsize), key='-SMLGDISP-'),
               sg.Text('Selected Name: Small Name', font=("Helvetica", fontsize), key='-SELECTEDNAME-')],
              [sg.Text('')],
              [sg.Button('Go Back'), sg.Button('Continue to Abilities')]
              ]

   nameentrysize = (15,1)
   topleft = [[sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-A1CUSTNAME-')],
              [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-A2CUSTNAME-')],
              [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-A3CUSTNAME-')],
              [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-A4CUSTNAME-')]]
   topleftgroup = [sg.Frame('', topleft)]
   botleft = [[sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-A5CUSTNAME-')],
              [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-A6CUSTNAME-')],
              [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-A7CUSTNAME-')],
              [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-A8CUSTNAME-')]]
   botleftgroup = [sg.Frame('', botleft)]
   left = sg.Column([topleftgroup, botleftgroup])
   leftgroup = [[sg.Frame('', [[left]])]]

   topmidleft = [[sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-B1CUSTNAME-')],
                 [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-B2CUSTNAME-')],
                 [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-B3CUSTNAME-')],
                 [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-B4CUSTNAME-')]]
   topmidleftgroup = [sg.Frame('', topmidleft)]
   botmidleft = [[sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-B5CUSTNAME-')],
                 [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-B6CUSTNAME-')],
                 [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-B7CUSTNAME-')],
                 [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-B8CUSTNAME-')]]
   botmidleftgroup = [sg.Frame('', botmidleft)]
   midleft = sg.Column([topmidleftgroup, botmidleftgroup])
   midleftgroup = [[sg.Frame('', [[midleft]])]]

   topmidright = [[sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-C1CUSTNAME-')],
                  [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-C2CUSTNAME-')],
                  [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-C3CUSTNAME-')],
                  [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-C4CUSTNAME-')]]
   topmidrightgroup = [sg.Frame('', topmidright)]
   botmidright = [[sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-C5CUSTNAME-')],
                  [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-C6CUSTNAME-')],
                  [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-C7CUSTNAME-')],
                  [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-C8CUSTNAME-')]]
   botmidrightgroup = [sg.Frame('', botmidright)]
   midright = sg.Column([topmidrightgroup, botmidrightgroup])
   midrightgroup = [[sg.Frame('', [[midright]])]]

   topright = [[sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-D1CUSTNAME-')],
               [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-D2CUSTNAME-')],
               [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-D3CUSTNAME-')],
               [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-D4CUSTNAME-')]]
   toprightgroup = [sg.Frame('', topright)]
   botright = [[sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-D5CUSTNAME-')],
               [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-D6CUSTNAME-')],
               [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-D7CUSTNAME-')],
               [sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-D8CUSTNAME-')]]
   botrightgroup = [sg.Frame('', botright)]
   right = sg.Column([toprightgroup, botrightgroup])
   rightgroup = [[sg.Frame('', [[right]])]]

   abilnameenterlayout = [sg.Column(leftgroup), sg.Column(midleftgroup), sg.Column(midrightgroup),
                          sg.Column(rightgroup)]

   layout4 = [[sg.Text('Enter ability names in the location you would like them to appear (leave slots with no ability blank).', font=("Helvetica", fontsize))],
              [sg.Text('Abilities must have unique names. Only enter Offensive Abilities.')],
              abilnameenterlayout,
              [sg.Text('', size=(55,1)), sg.InputText('', size=nameentrysize, font=("Helvetica", fontsize), k='-ECUSTNAME-')],
              [sg.Text('')],
              [sg.Button('Go Back'), sg.Button('Continue to Roll Objectives')]
              ]

   layout5 = [[sg.Text('Set Roll Objective', font=("Helvetica", fontsize + 10))],
              [sg.Text('Ability Name:', font=("Helvetica", fontsize)), sg.Text(custabilnames[0], font=("Helvetica", fontsize), k='-RONAME-')],
              [sg.Text('Current Roll Objective:', font=("Helvetica", fontsize)), sg.Text(custrollobjs[0], font=("Helvetica", fontsize), k='-ROOBJ-')],
              [sg.Text('Roll Objective:', font=("Helvetica", fontsize)),
               sg.Spin(values=custdiceanat, readonly=True, text_color='black', size=(2,1), font=("Helvetica", fontsize), k='-RO1-'),
               sg.Spin(values=custdiceanat, readonly=True, text_color='black', size=(2,1), font=("Helvetica", fontsize), k='-RO2-'),
               sg.Spin(values=custdiceanat, readonly=True, text_color='black', size=(2,1), font=("Helvetica", fontsize), k='-RO3-'),
               sg.Spin(values=custdiceanat, readonly=True, text_color='black', size=(2,1), font=("Helvetica", fontsize), k='-RO4-'),
               sg.Spin(values=custdiceanat, readonly=True, text_color='black', size=(2,1), font=("Helvetica", fontsize), k='-RO5-'),
               sg.Button('Set As Roll Objective')],
              [sg.Text('Ability 1 of 1', k='-1OF1-')],
              [sg.Button('Previous Ability'), sg.Button('Next Ability')],
              [sg.Text('')],
              [sg.Button('Go Back'), sg.Button('Finish and Calc Default Odds')]
              ]

   savelayout = [[sg.Text('Save As', size=(14,1)), sg.InputText(size=(25,1), key='-SAVENAME-')],
                 [sg.Text('Save Location', size=(14,1)), sg.In(size=(25, 1), enable_events=True, key='-FOLDER-'), sg.FolderBrowse()]]
   layout6 = [[sg.Column(savelayout, element_justification='l')], [sg.Button('Save'), sg.Button('Continue Without Saving')]]


   # ----------- Create actual layout using Columns and a row of Buttons
   layout = [[sg.Column(layout1, key='-COL1-'), sg.Column(layout2, visible=False, key='-COL2-'),
              sg.Column(layout3, visible=False, key='-COL3-'), sg.Column(layout4, visible=False, key='-COL4-'),
              sg.Column(layout5, visible=False, key='-COL5-'), sg.Column(layout6, visible=False, key='-COL6-'),]]

   global vislayout
   vislayout = 1
   return sg.Window('Create Homebrew Hero', layout, relative_location=(0, 0),
                    finalize=True, resizable=False, element_justification='center', keep_on_top=True)


def make_win4(location, otherwindowsize):
   load = [[sg.Text('File'), sg.In(size=(25, 1), enable_events=True, key='-FILE-'), sg.FileBrowse()]]
   layout = [[sg.Column(load, element_justification='c')],
             [sg.Button('Load')],
             ]
   return sg.Window('Load Homebrew Hero', layout, location=(location[0]+(otherwindowsize[0]/2)-125,location[1]+\
                                                      (otherwindowsize[1]/2)-31), resizable=True, keep_on_top=True, finalize=True)


def centered_popup(content):
   windowxy = window.CurrentLocation()
   tempwindowsize = window.current_size_accurate()
   windowsize = []
   windowsize.append(int(tempwindowsize[0]))
   windowsize.append(int(tempwindowsize[1]))
   return  sg.popup(content, location=(int(windowxy[0] + (windowsize[0] / 2) - 100), int(windowxy[1] +
                                        (windowsize[1] / 2) - 31)), keep_on_top = True)


window1, window2 = make_win1(), None
window3 = None
window4 = None
Barbarian.set_as_active()

helppopup = sg.popup('Real Time DT Odds is a training tool to help you see your chances of getting different abilities at '
                'any point during your Offensive Roll Phase.',
                'See the Help menu for more information.',
                keep_on_top=True)

editing = False
firsttime = True

def calcMaxCP():
    cp = 0;
    cards = 0;
    cardlvl= int(window['-SIXIT-'].get())
    if(cardlvl >= 1):
        cards += 1
    if(cardlvl == 1):
        cp += 1
    cardlvl = int(window['-SAMESIES-'].get())
    if(cardlvl >= 1):
        cards += 1
    if(cardlvl == 1):
        cp += 1
    if(cardlvl == 2):
        cp += 0
    if(cardlvl == 3):
        cp += 2
    if(cardlvl >= 4):
        cp += 2
    cardlvl = int(window['-TIPIT-'].get())
    if(cardlvl >= 1):
        cards += 1
    if(cardlvl == 1):
        cp += 1
    if(cardlvl == 2):
        cp += 0
    if(cardlvl == 3):
        cp += 0
    if(cardlvl >= 4):
        cp += 0
    cardlvl = int(window['-WILD-'].get())
    if(cardlvl >= 1):
        cards += 1
    if(cardlvl == 1):
        cp += 2
    if(cardlvl == 2):
        cp += 1
    if(cardlvl == 3):
        cp += 0
    if(cardlvl >= 4):
        cp += 0
    cardlvl = int(window['-2WILD-'].get())
    if(cardlvl >= 1):
        cards += 1
    if(cardlvl == 1):
        cp += 3
    if(cardlvl == 2):
        cp += 2
    if(cardlvl == 3):
        cp += 1
    if(cardlvl >= 4):
        cp += 0
    cardlvl = int(window['-SWILD-'].get())
    if(cardlvl >= 1):
        cards += 1
    if(cardlvl == 1):
        cp += 1
    if(cardlvl == 2):
        cp += 1
    if(cardlvl == 3):
        cp += 0
    if(cardlvl >= 4):
        cp += 0
    window1['-MAXCPTEXT-'].update("CP (" + str(cp) + "):")
    window1['-MAXCARDSTEXT-'].update("Cards (" + str(cards) + "):")

while True:             # Event Loop
   window, event, values = sg.read_all_windows()
   if event == sg.WIN_CLOSED or event == 'Exit':
       window.close()
       if window == window2:       # if closing win 2, mark as closed
           window2 = None
       elif window == window3:
           window3 = None
       elif window == window4:
           window4 = None
       elif window == window1:     # if closing win 1, exit program
           break
   elif event == '-NORMSIM-' and not window2:
       if window['-RA-'].get() == '0' and window['-RR-'].get() == '0':
           centered_popup('There must be at least 1 Roll Attempt or 1 Re-Roll for there to be anything to simulate.')
       else:
           window1['-RARed-'].update(visible = False)
           window1['-RA-'].update(visible = True)
           windowxy = window.CurrentLocation()
           tempwindowsize = window.current_size_accurate()
           windowsize = []
           windowsize.append(int(tempwindowsize[0]))
           windowsize.append(int(tempwindowsize[1]))
           window2 = make_win2(windowxy, windowsize)
           progress_bar = window2['-PROGRESS BAR-']
           progress_bar.update(current_count=1)
           #set parameters
           curentdicevalues = window['-D1-'].get()+window['-D2-'].get()+window['-D3-'].get()+\
                              window['-D4-'].get()+window['-D5-'].get()
           rerolls = int(window['-RR-'].get())
           rollatmps = int(window['-RA-'].get())
           #wilds = window['-W-'].get()
           returnstring = run_norm_sim_exe("false",curentdicevalues, rollatmps, rerolls)
           print_results2(returnstring, curentdicevalues)
           #normresdict = run_norm_sim()
           #print_results(normresdict)
           whatsettingswereused = 'Starting Dice: ' + str(curentdicevalues) + ' | ' \
                                  + 'Chased: All | Roll Attempts: ' + str(rollatmps) + ' | Re-Rolls: ' + str(rerolls)
           window1['-DISPLAYING-'].update(whatsettingswereused)
           window1['-DISPLAYING2-'].update(getCardsString())
           window1['-RARed-'].update(visible = False)
           window1['-RA-'].update(visible = True)
           window2.close()
           window2 = None
   elif event == '-CHASESIM-':
       if window['-RA-'].get() == '0' and window['-RR-'].get() == '0':
           centered_popup('There must be at least 1 Roll Attempt or 1 Re-Roll for there to be anything to simulate.')
       else:
           window1['-RARed-'].update(visible = False)
           window1['-RA-'].update(visible = True)
           windowxy = window.CurrentLocation()
           tempwindowsize = window.current_size_accurate()
           windowsize = []
           windowsize.append(int(tempwindowsize[0]))
           windowsize.append(int(tempwindowsize[1]))
           window2 = make_win2(windowxy, windowsize)
           progress_bar = window2['-PROGRESS BAR-']
           progress_bar.update(current_count=1)
           #set parameters
           curentdicevalues = window['-D1-'].get()+window['-D2-'].get()+window['-D3-'].get()+\
                              window['-D4-'].get()+window['-D5-'].get()
           rerolls = int(window['-RR-'].get())
           rollatmps = int(window['-RA-'].get())
           wilds = window['-WILD-'].get()
           objname = window['-CHASE-'].get()
           if objname == smallname:
               chaseobj = ["SMALL"]
           elif objname == largename:
               chaseobj = ["BIG"]
           else:
               chaseobj = rollobjs[abilitynames.index(objname)]
           print("chasedobj",chaseobj)
           returnstring = run_fallback_sim_exe(chaseobj, "false",curentdicevalues, rollatmps, rerolls)
           print_results2(returnstring, curentdicevalues)
           whatsettingswereused = 'Starting Dice: ' + str(curentdicevalues) + ' | ' +\
                                  'Chased: ' + objname + ' | Roll Attempts: ' + str(rollatmps) +\
                                  ' | Re-Rolls: ' + str(rerolls)
           window1['-DISPLAYING-'].update(whatsettingswereused)
           window1['-DISPLAYING2-'].update(getCardsString())
           window2.close()
           window2 = None
   elif event == '-DEFAULTODDS-':
       #if isinstance(hero.defaultodds, int):
       #    defaultdict = dict.fromkeys(allabilitynames, hero.defaultodds)
       #else:
       #    defaultdict = dict(zip(allabilitynames, hero.defaultodds))
       #print_results(defaultdict)
       window1['-RARed-'].update(visible = True)
       window1['-RA-'].update(visible = False)
       curentdicevalues = window['-D1-'].get()+window['-D2-'].get()+window['-D3-'].get()+\
                              window['-D4-'].get()+window['-D5-'].get()
       rerolls = int(window['-RR-'].get())
       rollatmps = int(window['-RA-'].get())
       returnstring = run_norm_sim_exe("true",curentdicevalues, rollatmps, rerolls)
       print_results2(returnstring, "")
       whatsettingswereused = 'Default Odds ' + ' | Roll Attempts: ' + str(rollatmps) +\
                                  ' | Re-Rolls: ' + str(rerolls)
       window1['-DISPLAYING-'].update(whatsettingswereused)
       window1['-DISPLAYING2-'].update(getCardsString())
       
   elif event == 'Create Homebrew' and not window3:
       reset_custom()
       window3 = make_win3()
       editing = False
   elif event == 'Edit Current Hero' and not window3:
       reset_custom()
       window3 = make_win3()
       editing = True
       firsttime = True
       window3['-CUSTNAME-'].update(value=hero.name)
       custabilnames = hero.abilnames
       custabilitylocate = hero.abilitylocate
   elif window3:
       if event == 'Continue to Straights':
           for number in range(1,7):
               custdiceanat[number-1] = window[f'-CD{number}-'].get()
           if editing:
               custsmlocate = hero.smlocate
               custlglocate = hero.lglocate
               custsmname = hero.smname
               custlgname = hero.lgname
               window['-SELECTEDPOS-'].update('Selected Position: ' + custsmlocate)
               window['-STNAME-'].update(custsmname)
               window['-SELECTEDNAME-'].update('Selected Name: ' + custsmname)
           window[f'-COL{vislayout}-'].update(visible=False)
           vislayout = vislayout + 1 if vislayout < 6 else 1
           window[f'-COL{vislayout}-'].update(visible=True)
       elif event == 'Continue to Dice Anatomy':
           if editing:
               for numb in range(1,7):
                   window[f'-CD{numb}-'].update(value=hero.diceanat[numb-1])
           else:
               for numb in range(1,7):
                   window[f'-CD{numb}-'].update(value=custdiceanat[numb-1])
           custname = window['-CUSTNAME-'].get()
           window[f'-COL{vislayout}-'].update(visible=False)
           vislayout = vislayout + 1 if vislayout < 6 else 1
           window[f'-COL{vislayout}-'].update(visible=True)
       elif event == '-SETTYPE-':
           window['-STNAMEPROMPT-'].update('Enter '+window['-STDROP-'].get()+' Name')
           window['-STLOCATIONPROMPT-'].update('Select ' + window['-STDROP-'].get() + ' Location')
           window['-SMLGDISP-'].update(window['-STDROP-'].get())
           if "Small" in window['-STNAMEPROMPT-'].get():
               window['-SELECTEDPOS-'].update('Selected Position: ' + custsmlocate)
               window['-STNAME-'].update(custsmname)
               window['-SELECTEDNAME-'].update('Selected Name: ' + custsmname)
           elif "Large" in window['-STNAMEPROMPT-'].get():
               window['-SELECTEDPOS-'].update('Selected Position: ' + custlglocate)
               window['-STNAME-'].update(custlgname)
               window['-SELECTEDNAME-'].update('Selected Name: ' + custlgname)
       elif event == '-SETNAME-':
           if "Small" in window['-STNAMEPROMPT-'].get():
               custsmname = window['-STNAME-'].get()
               window['-SELECTEDNAME-'].update('Selected Name: ' + custsmname)
               #window['-STNAME-'].update(custsmname)
           elif "Large" in window['-STNAMEPROMPT-'].get():
               custlgname = window['-STNAME-'].get()
               window['-SELECTEDNAME-'].update('Selected Name: ' + custlgname)
               #window['-STNAME-'].update(custlgname)
       elif event in everylocation:
           if "Small" in window['-STNAMEPROMPT-'].get():
               custsmlocate = event
               window['-SELECTEDPOS-'].update('Selected Position: '+event)
           elif "Large" in window['-STNAMEPROMPT-'].get():
               custlglocate = event
               window['-SELECTEDPOS-'].update('Selected Position: '+event)
       elif event == 'Continue to Abilities':
           for col in ['A','B','C','D']:
               for row in range(1,9):
                   window[f'-{col}{row}CUSTNAME-'].update(text_color='white', disabled=False)
                   if f'{col}{row}' in custabilitylocate:
                       window[f'-{col}{row}CUSTNAME-'].update(value=custabilnames[custabilitylocate.index(f'{col}{row}')])
                   else:
                       window[f'-{col}{row}CUSTNAME-'].update(value='')
           if 'E' in custabilitylocate:
               window['-ECUSTNAME-'].update(value=custabilnames[custabilitylocate.index('E')])
           window[f'-{custsmlocate}CUSTNAME-'].update(value=custsmname, text_color='black', disabled=True)
           window[f'-{custlglocate}CUSTNAME-'].update(value=custlgname, text_color='black', disabled=True)
           window[f'-COL{vislayout}-'].update(visible=False)
           vislayout = vislayout + 1 if vislayout < 6 else 1
           window[f'-COL{vislayout}-'].update(visible=True)
       elif event == 'Continue to Roll Objectives':
           abilnamebackup = custabilnames.copy()
           abillocatebackup = custabilitylocate.copy()
           custabilitylocate = []
           custabilnames = []
           for col in ['A','B','C','D']:
               for row in range(1,9):
                   if window[f'-{col}{row}CUSTNAME-'].get() not in ['', custsmname, custlgname]:
                       custabilitylocate.append(f'{col}{row}')
                       custabilnames.append(window[f'-{col}{row}CUSTNAME-'].get())
           if window['-ECUSTNAME-'].get() != '':
               custabilitylocate.append('E')
               custabilnames.append(window['-ECUSTNAME-'].get())
           for namecheck in abilnamebackup:
               if namecheck not in custabilnames:
                   custrollobjs.pop(abilnamebackup.index(namecheck))
           for namecheck in custabilnames:
               if namecheck not in abilnamebackup:
                   custrollobjs.insert(custabilnames.index(namecheck), '')
           if len(custabilnames) > 0 and window['-ECUSTNAME-'].get() != '':
               custabilnameindex = 0
               window['-RONAME-'].update(value=custabilnames[custabilnameindex])
               if custrollobjs[custabilnameindex] != '' and custrollobjs[custabilnameindex] != 'ZZZ':
                   window['-ROOBJ-'].update(value=custrollobjs[custabilnameindex])
               else:
                   window['-ROOBJ-'].update(value='')
               faceselection = custdiceanat.copy()
               faceselection = list(set(faceselection))
               faceselection.append('')
               faceselection.sort()
               for die in range(1,6):
                   window[f'-RO{die}-'].update(values=faceselection)
                   window[f'-RO{die}-'].update(value=faceselection[0])
               if custrollobjs[0] == 'ZZZ' or len(custrollobjs) != len(custabilnames):
                   custrollobjs = []
                   for abilname in custabilnames:
                       custrollobjs.append('')
               if editing:
                   if firsttime:
                       firsttime = False
                       for name in custabilnames:
                           if name in hero.abilnames:
                               heronameindex = hero.abilnames.index(name)
                               thisnamesrollobj = hero.rollobjectives[heronameindex]
                               custnameindex = custabilnames.index(name)
                               tempobj = []
                               for letter in thisnamesrollobj:
                                   tempobj.append(letter)
                               tempobj.sort()
                               custrollobjs[custnameindex] = tempobj
                   if custabilnames[0] in hero.abilnames:
                       window['-ROOBJ-'].update(value=custrollobjs[0])
               window['-1OF1-'].update(value=f'Ability {custabilnameindex+1} of {len(custabilnames)}')
               window[f'-COL{vislayout}-'].update(visible=False)
               vislayout = vislayout + 1 if vislayout < 6 else 1
               window[f'-COL{vislayout}-'].update(visible=True)
           else:
               centered_popup('Must have an Ultimate and at least 1 other non-straight ability.')
       elif event == 'Set As Roll Objective':
           thisrobj = []
           for die in range(1, 6):
               if window[f'-RO{die}-'].get() != '':
                   thisrobj.append(window[f'-RO{die}-'].get())
               thisrobj.sort()
           custrollobjs[custabilnameindex] = thisrobj
           window['-ROOBJ-'].update(value=custrollobjs[custabilnameindex])
       elif event == 'Previous Ability':
           if custabilnameindex > 0:
               custabilnameindex -= 1
               window['-RONAME-'].update(value=custabilnames[custabilnameindex])
               window['-ROOBJ-'].update(value=custrollobjs[custabilnameindex])
               window['-1OF1-'].update(value=f'Ability {custabilnameindex+1} of {len(custabilnames)}')
           else:
               pass
       elif event == 'Next Ability':
           if custabilnameindex < len(custabilnames)-1:
               custabilnameindex += 1
               window['-RONAME-'].update(value=custabilnames[custabilnameindex])
               window['-ROOBJ-'].update(value=custrollobjs[custabilnameindex])
               window['-1OF1-'].update(value=f'Ability {custabilnameindex+1} of {len(custabilnames)}')
           else:
               pass
       elif event == 'Finish and Calc Default Odds':
           allgood = True
           for check in custrollobjs:
               if check == '' or check == []:
                   allgood = False
                   break
           if allgood:
               customhero = Hero(custname, custdiceanat, custabilnames, custrollobjs, custabilitylocate, custsmname, custsmlocate, custlgname, custlglocate)
               setHero(customhero)
               windowxy = window.CurrentLocation()
               tempwindowsize = window.current_size_accurate()
               windowsize = []
               windowsize.append(int(tempwindowsize[0]))
               windowsize.append(int(tempwindowsize[1]))
               window2 = make_win2(windowxy, windowsize)
               progress_bar = window2['-PROGRESS BAR-']
               progress_bar.update(current_count=1)
               #defoddsdict = find_default_odds()
               defaultodds = []
               customhero = Hero(custname, custdiceanat, custabilnames, custrollobjs, custabilitylocate, custsmname, custsmlocate, custlgname, custlglocate, defaultodds)
               setHero(customhero)
               window2.close()
               window2 = None
               window['-SAVENAME-'].update(value=custname)
               window[f'-COL{vislayout}-'].update(visible=False)
               vislayout = vislayout + 1 if vislayout < 6 else 1
               window[f'-COL{vislayout}-'].update(visible=True)
           else:
               centered_popup('All abilities must have a roll objective.')
       elif event == 'Continue Without Saving':
           window3.close()
           window3 = None
       elif event == 'Save':
           folder = values['-FOLDER-']
           savelocation = folder + '/' + window['-SAVENAME-'].get() + '.dto'
           try:
               with open(savelocation, 'wb') as savefile:
                   pickle.dump(customhero, savefile)
               window3.close()
               window3 = None
           except:
               centered_popup('Please select a valid save location and name.')
       elif 'Go Back' in event:
           window[f'-COL{vislayout}-'].update(visible=False)
           vislayout = vislayout - 1 if vislayout > 1 else 1
           window[f'-COL{vislayout}-'].update(visible=True)
   elif event == 'Load Homebrew':
       windowxy = window.CurrentLocation()
       tempwindowsize = window.current_size_accurate()
       windowsize = []
       windowsize.append(int(tempwindowsize[0]))
       windowsize.append(int(tempwindowsize[1]))
       window4 = make_win4(windowxy, windowsize)
   elif event == 'Load':
       filename = values['-FILE-']
       if '.dto' in filename:
           try:
               with open(filename, 'rb') as loadfile:
                   customhero = pickle.load(loadfile)
               customhero.set_as_active()
               window4.close()
               window4 = None
           except:
               centered_popup('Please select a .dto file.')
       else:
           centered_popup('Please select a .dto file.')
   elif event == '-RA-':
       sixit = window1['-RA-'].get()
       window1["-RARed-"].update(value = sixit)
   elif event == '-RARed-':
       sixit = window1['-RARed-'].get()
       window1["-RA-"].update(value = sixit)
   elif event == '-SIXIT-':
       sixit = window['-SIXIT-'].get()
       calcMaxCP()
   elif event == '-SAMESIES-':
       samesies = window['-SAMESIES-'].get()
       calcMaxCP()
   elif event == '-TIPIT-':
       calcMaxCP()
   elif event == '-WILD-':
       calcMaxCP()
   elif event == '-2WILD-':
       calcMaxCP()
   elif event == '-SWILD-':
       calcMaxCP()
   elif event == '-CHEER-':
       calcMaxCP()
   elif event == '-PROBMANI-':
       calcMaxCP()
   elif event == '-A1CHASE-':
       chaseability = 'A1'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-A2CHASE-':
       chaseability = 'A2'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-A3CHASE-':
       chaseability = 'A3'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-A3CHASE-':
       chaseability = 'A3'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-A4CHASE-':
       chaseability = 'A4'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-A5CHASE-':
       chaseability = 'A5'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-A6CHASE-':
       chaseability = 'A6'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-A7CHASE-':
       chaseability = 'A7'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-A8CHASE-':
       chaseability = 'A8'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-B1CHASE-':
       chaseability = 'B1'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-B2CHASE-':
       chaseability = 'B2'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-B3CHASE-':
       chaseability = 'B3'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-B4CHASE-':
       chaseability = 'B4'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-B5CHASE-':
       chaseability = 'B5'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-B6CHASE-':
       chaseability = 'B6'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-B7CHASE-':
       chaseability = 'B7'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-B8CHASE-':
       chaseability = 'B8'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-C1CHASE-':
       chaseability = 'C1'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-C2CHASE-':
       chaseability = 'C2'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-C3CHASE-':
       chaseability = 'C3'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-C4CHASE-':
       chaseability = 'C4'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-C5CHASE-':
       chaseability = 'C5'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-C6CHASE-':
       chaseability = 'C6'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-C7CHASE-':
       chaseability = 'C7'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-C8CHASE-':
       chaseability = 'C8'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-D1CHASE-':
       chaseability = 'D1'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-D2CHASE-':
       chaseability = 'D2'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-D3CHASE-':
       chaseability = 'D3'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-D4CHASE-':
       chaseability = 'D4'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-D5CHASE-':
       chaseability = 'D5'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-D6CHASE-':
       chaseability = 'D6'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-D7CHASE-':
       chaseability = 'D7'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-D8CHASE-':
       chaseability = 'D8'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == '-ECHASE-':
       chaseability = 'E'
       window['-CHASE-'].Update(name_from_address(chaseability))
   elif event == 'Barbarian':
       Barbarian.set_as_active()
   elif event == 'Monk':
       Monk.set_as_active()
   elif event == 'Moon Elf':
       MoonElf.set_as_active()
   elif event == 'Ninja':
       Ninja.set_as_active()
   elif event == 'Paladin':
       Paladin.set_as_active()
   elif event == 'Pyromancer':
       Pyromancer.set_as_active()
   elif event == 'Shadow Thief':
       ShadowThief.set_as_active()
   elif event == 'Treant':
       Treant.set_as_active()
   elif event == 'Artificer':
       Artificer.set_as_active()
   elif event == 'Cursed Pirate':
       CursedPirate.set_as_active()
   elif event == 'Gunslinger':
       Gunslinger.set_as_active()
   elif event == 'Huntress':
       Huntress.set_as_active()
   elif event == 'Samurai':
       Samurai.set_as_active()
   elif event == 'Seraph':
       Seraph.set_as_active()
   elif event == 'Tactician':
       Tactician.set_as_active()
   elif event == 'Vampire Lord':
       VampireLord.set_as_active()
   elif event == 'Black Panther':
       BlackPanther.set_as_active()
   elif event == 'Black Widow':
       BlackWidow.set_as_active()
   elif event == 'Captain Marvel':
       CaptainMarvel.set_as_active()
   elif event == 'Dr Strange':
       pass
       DrStrange.set_as_active()
   elif event == 'Loki':
       pass
       Loki.set_as_active()
   elif event == 'Scarlet Witch':
       pass
       ScarletWitch.set_as_active()
   elif event == 'Spider-Man':
       pass
       SpiderMan.set_as_active()
   elif event == 'Thor':
       Thor.set_as_active()
   elif event == 'Low (Fast)':
       simcount = 1000
       centered_popup('Sim Accuracy set to: Low (Fast)')
   elif event == 'Medium (Medium)':
       simcount = 5000
       centered_popup('Sim Accuracy set to: Medium (Medium)')
   elif event == 'High (Slow)':
       simcount = 10000
       centered_popup('Sim Accuracy set to: High (Slow)')
   elif event == 'About...':
       centered_popup('Real Time DT Odds is a training tool to help you see your chances of getting different abilities at '
                'any point during your Offensive Roll Phase.\n\nGUI Made by Assassinater#9873 on Discord.')
   elif event == 'Chase Definition':
       centered_popup('Chase Definition:\nTo "chase" an ability, means to re-roll every die that does not match the roll objective for that'
                ' ability. It also means that you do not keep extras of a symbol if you do not absolutely need them to'
                ' activate the ability. The straights are chased the same way minions and bosses chase straights in DTA, which is to '
                'always re-roll 1s and 6s unless they are already being used as part of a small or large straight.')
   elif event == 'How to Use':
       centered_popup('Enter the values of your current dice, how many Roll Attempts you have left, how many Re-Rolls '
                'you have available, and how many wilds you have. \nIf you have the card One More Time and can afford it'
                ' and are willing to play it, '
                'you can factor it in by adding 1 Roll Attempt, the same applies for the card Try Try Again except you '
                'would add 2 Re-Rolls. \n\nThe Default Odds button will display the odds for each ability as if you chased it, assuming you have '
                'just started your Offensive Roll Phase. Meaning you have 3 Roll Attempts and your dice have no '
                'starting values. \n\nThe Chase All Simulate button will display the odds for each ability as if you chased it, taking into '
                'account your current dice, Roll Attempts, Re-Rolls, Wilds, Six-It, and Samesies. \n\nThe Chase *Ability Name* Simulate button will display the odds for your chased ability as if you '
                'chased it. The odds displayed for all other abilities are the chances you stumble into those abilities '
                'while chasing your chased ability. It is assumed you immediately stop rolling once you get your chased '
                'ability. \n\nYou can change the Hero with the menu in the top left as well as the simulation accuracy (which '
                'effects the speed).\n\nYou can create Homebrew heroes through the Homebrew menu. You can also load in '
                'previously created Homebrews.')
   elif event == 'Other Notes':
       centered_popup('When using Wilds/Six-It/Samesies (cards) and chasing a specific ability, it is assumed you stop '
                'rolling as soon as you get the '
                'ability you are chasing. If you do not get it by using all your Roll Attempts and Re-Rolls, '
                'then your cards will be used. \n\nFor the abilities you are not chasing, it is assumed you are'
                ' willing to spend your cards to get them instead of your chased ability.'
                '\n\nIt is assumed that when you keep multiple of the same symbol, you will keep different nubmers if possible.'
                '\n\nThe odds displayed will very slightly everytime you run simulation, but they are generally within 1-2%. '
                'The odds are also rounded, so if it says 0% you may technically have a chance, but that chance is '
                'less than 0.5%.'
                '\n\nWhy is there no Tip-It option? Because coding is hard.')
window.close()
