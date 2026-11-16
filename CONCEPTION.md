# Désamorçage coopératif : document de conception

Projet *OS embarqués pour l'Edge Computing* (SI5/M2, 2026-2027).
Smart sensor/actuator Arduino Uno + FreeRTOS, passerelle Raspberry Pi (maître I2C), service cloud.

---

## 0. Ce qu'il faut savoir avant de lire

L'idée de départ s'appuyait sur des capteurs **absents de notre kit** : IMU, capteur de bruit, capteur de couleur, capteur tactile, NFC, anneau de LED, émetteur 433 MHz. Ce document reprend le concept avec le matériel de `components.txt` :

| Capteurs | Actionneurs | Infra |
|---|---|---|
| Télémètre à ultrasons | Écran LCD 16x2 (I2C) | Arduino Uno |
| Potentiomètre rotatif | Afficheur 4 chiffres (TM1637) | Grove Base Shield v2 |
| Capteur de lumière v1.2 | LED Bar 10 segments (MY9221) | Câbles jumper (Grove ↔ broches, RPi) |
| 3 boutons Grove | LED rouge, verte, bleue | (Raspberry Pi, fourni à part) |
| Interrupteur DIP 6 positions (I2C) | Buzzer | |
| Keypad 3×4 tout fait (0–9, `*`, `#`, hors Grove), via un module PCF8574 | | |

L'**émetteur et le récepteur infrarouges** du kit ne sont **pas utilisés**. L'émetteur exige D3 et le Timer2, pris par le buzzer, il n'y a plus de broches, et ils portaient le module le plus risqué (une barrière IR). Tous les autres composants ont un rôle précis.

**Correspondance avec l'idée d'origine**

| Idée d'origine | Remplacé par | Ce qui change |
|---|---|---|
| Stabilisateur (IMU, inclinaison > 20°) | **Périmètre** : ultrasons, zone interdite autour de « l'œil » de la bombe | C'est ce module qui porte la contrainte dure de 50 ms |
| Silence (capteur de bruit) | **Occultation** : capteur de lumière, à masquer N fois | Même esprit (« faire un geste au bon moment ») |
| Cartouches (capteur de couleur) | *retiré* | Pas de capteur de couleur ; la variante à barrière IR a été abandonnée au profit du clavier |
| Clavier (tactile) | **Clavier** : keypad 3×4 tout fait, relié au bus I2C par un PCF8574 | Un code déduit d'une règle du manuel, comme dans l'idée d'origine |
| Clé NFC | **Fréquence** : potentiomètre et LCD | Le n° de série affiché reste au cœur des règles |
| *(nouveau)* | **Relais** : 3 boutons Grove sous 3 LED de couleur | Module de mémoire en 3 étapes |
| *(nouveau)* | **Détonateur** : interrupteur DIP 6 positions | Code binaire à composer, interrupteurs piégés |
| Afficheur du compte à rebours | **Afficheur 4 chiffres** `MM:SS`, puis `SS:cc` sous la minute | Le chrono « de film », toujours visible |
| Anneau LED (fautes, état) | **LED Bar** : jauge de stabilité | Se vide à chaque faute ; devient la mèche pendant la finale |
| Bouton LED « fil rouge » | **Finale** : maintenir le bon bouton, relâcher au bon chiffre | |
| Lampe 433 MHz | Buzzer (vraies notes), stroboscope des LED, son d'explosion joué par le RPi | |

---

## 1. Le jeu

**Rôles**

- **Démineur** : a la bombe entre les mains. Il voit le LCD, l'afficheur, les LED, entend le buzzer. N'a pas le manuel.
- **Expert** : a le manuel sur son téléphone (page web générée par le cloud). Ne voit pas la bombe.
- **Saboteur** (optionnel) : console web à distance. Peut accélérer le chrono, brouiller l'affichage ou déclencher une fausse alarme, avec un budget limité.

**Principe anti-mémorisation** : pour chaque partie, le cloud tire une **graine**. À partir de cette graine, un générateur pseudo-aléatoire déterministe produit à la fois :

1. le **manuel** (tables, mots, ordre des modules, conditions) envoyé au téléphone de l'expert ;
2. la **configuration** de la bombe (stimuli à afficher + réponses attendues) envoyée à l'Arduino via MQTT → RPi → I2C. Pendant le transfert, la LED Bar se remplit page par page.

L'Arduino ne connaît **pas les règles**, seulement les stimuli et les réponses attendues. Le firmware reste petit (2 Ko de RAM), et la solution ne sort jamais de la bombe.

**Déroulement**

1. L'équipe crée une partie sur le site (niveau choisi). Le cloud génère graine, manuel et configuration.
2. La configuration descend jusqu'à la bombe. Le LCD affiche `ARMEE` et le **numéro de série** (ex. `KX4-7B2`), l'afficheur 4 chiffres la durée (`05:00`).
3. Tant qu'un interrupteur DIP est sur ON, le LCD affiche `DIP -> OFF` et la bombe refuse de démarrer. Une fois tout à OFF, le démineur appuie 1 s sur les 3 boutons Grove à la fois : le chrono démarre.
4. Le module **Périmètre** est actif en permanence. Les autres modules s'enchaînent dans l'ordre indiqué par le manuel ; le LCD affiche le module en cours.
5. Tous les modules sont résolus : la **finale** (fil rouge). Réussie : `DESAMORCEE`. Chrono à 0 ou trop de fautes : `BOOM`.
6. Le score part au classement.

**Retours pendant la partie**

Le LCD est entièrement réservé aux consignes des modules : le chrono est sur l'afficheur 4 chiffres et les fautes sur la LED Bar. Le Timer2 étant libre, le buzzer joue de **vraies notes** avec `tone()` (hauteur variable, sirène, mélodie).

| Événement | Afficheur 4 chiffres | LED Bar | LCD | LED | Buzzer |
|---|---|---|---|---|---|
| Tic du chrono | `03:27`, deux-points qui clignotent ; `SS:cc` sous la minute | — | consigne du module | — | tic bref chaque seconde, puis 2/s sous 30 s |
| Faute | — | perd des segments | `FAUTE !` 1 s | flash rouge 300 ms | `BZZT` grave 200 ms |
| Module résolu | — | — | `OK` puis `MODULE 3/6` | flash vert | double bip aigu |
| Sabotage « accélération » | défile plus vite | — | rien (c'est le but) | — | les tics s'accélèrent : les joueurs l'**entendent** |
| Sabotage « brouillage » | segments au hasard pendant 5 s : **le chrono disparaît** | — | caractères au hasard | — | — |
| Explosion | `8888` qui clignote | stroboscope | `*** BOOM ***` | stroboscope des 3 LED | sirène montante 2 s |
| Victoire | temps restant figé | vert fixe | `DESAMORCEE` | vert fixe | mélodie |

Chaque faute accélère aussi le chrono de +10 % (réglable par niveau).

**La LED Bar, jauge de stabilité**

- Pleine au départ. Chaque faute éteint `10 / (fautes tolérées + 1)` segments. Vide : explosion.
- Pendant la finale, elle devient une **mèche** qui se consume au rythme du chrono.
- Niveau Recrue seulement (mode assisté) : elle clignote quand un objet approche à moins de 2 × D de l'œil du Périmètre.

---

## 2. Les modules

Pour chaque module : matériel, ce que voit le démineur, extrait de manuel **généré** (les valeurs changent à chaque graine), traitement local fait par l'Arduino.

### 2.1 Périmètre (ultrasons) : **contrainte temps réel dure**

- **Matériel** : télémètre à ultrasons sur la face supérieure (« l'œil » de la bombe). Actionneurs : LED rouge et buzzer.
- **Règle** : aucun objet ne doit entrer à moins de **D cm** de l'œil. Le seuil D vient du niveau et du manuel.
- **Extrait de manuel** : *« L'œil surveille une zone de 10 cm si le numéro de série se termine par un chiffre pair, 20 cm sinon. Toute intrusion est une faute. »*
- **Piège volontaire** : la cellule photosensible (module Occultation) est placée à côté de l'œil. Pour la masquer, il faut arriver **par le côté**, et seul le manuel le dit.
- **Exigence** : une intrusion est détectée et signalée (LED rouge + buzzer) en **moins de 50 ms**, quoi que fassent les autres tâches (LCD, I2C, saboteur…). Budget et mesure au §6.
- **Traitement local** : mesure du temps de vol par interruption (pas de `pulseIn` bloquant), confirmation sur 2 échantillons consécutifs pour éviter les faux positifs, rejet des valeurs aberrantes (< 2 cm).

### 2.2 Fréquence (potentiomètre + LCD + bouton)

- **Matériel** : potentiomètre (le « cadran »), LCD, bouton sous la LED verte pour valider.
- **Le démineur voit** : un mot en ligne 2 (ex. `ORAGE`) et la fréquence courante, mise à jour en direct quand il tourne le cadran (`3.40 MHz`, de 3.00 à 4.00 MHz par pas de 0.05).
- **Extrait de manuel** :

  > *Trouvez le mot affiché. Si le numéro de série contient une voyelle, lisez la colonne B, sinon la colonne A. Réglez le cadran puis appuyez sous la LED verte.*
  >
  > | Mot | A | B |
  > |---|---|---|
  > | ORAGE | 3.25 | 3.70 |
  > | ORANGE | 3.60 | 3.15 |
  > | CIGALE | 3.55 | 3.10 |
  > | SIGNAL | 3.05 | 3.85 |

  Les mots sont choisis **proches à l'oral** (ORAGE / ORANGE, CIGALE / SIGNAL) : les joueurs doivent épeler.
- **Traitement local** : moyenne glissante sur l'ADC, **hystérésis** entre crans (pas de valeur qui saute entre 3.40 et 3.45), quantification.

### 2.3 Occultation (capteur de lumière + LED bleue)

- **Matériel** : capteur de lumière (la « cellule »), LED bleue.
- **Le démineur voit** : la LED bleue clignote par salves de N éclairs, séparées par une pause.
- **Extrait de manuel** : *« Comptez les éclairs d'une salve (N). Si N + le dernier chiffre du n° de série est pair, masquez la cellule 2 fois. Sinon, masquez-la N fois. Si vous avez déjà fait une faute, ajoutez 1. »* La saisie est validée seule après 3 s de lumière continue.
- **Traitement local** (c'est du vrai *edge processing*) :
  - **calibration automatique** de la lumière ambiante à l'armement (moyenne sur 1 s) ;
  - seuil adaptatif à 50 % de l'ambiant, avec hystérésis ;
  - une occultation compte si elle dure **au moins 300 ms** (un passage de main rapide ne compte pas) ;
  - suivi lent de l'ambiant hors occultation (quelqu'un allume la lumière de la salle).

### 2.4 Clavier (keypad 3×4 : 0–9, `*`, `#`)

- **Matériel** : keypad 3×4 tout fait (matrice : 4 lignes, 3 colonnes, 7 fils), branché sur un **module PCF8574** (8 entrées/sorties pilotées en I2C), lui-même relié au hub du bus I2C logiciel. La sortie INT du PCF8574 arrive sur A2. Coût en broches de l'Arduino : **une seule (A2)**.
- **Le démineur voit** : `LOT 5829` en ligne 1 du LCD ; ce qu'il tape s'affiche en ligne 2 (`CODE: 31__`).
- **Extrait de manuel** :
  > *Prenez le numéro de lot. Si son dernier chiffre est pair, remplacez chaque chiffre par sa moitié (arrondie en dessous) ; sinon, inversez l'ordre des chiffres. Si le numéro de série contient une voyelle, ajoutez 1 au premier chiffre (9 devient 0). Tapez le code puis `#`. `*` efface.*

  **Variante « code croisé »** (niveau Expert) : *« Les deux premiers chiffres du code sont les décimales de la fréquence réglée au module Fréquence. »* Le démineur doit s'en souvenir : les modules ne sont plus indépendants.
- **Câblage** :
  - lignes du keypad sur P0–P3 du PCF8574, colonnes sur P4–P6 (P7 libre) ;
  - adresse du PCF8574 : 0x20 (ou 0x38–0x3F pour la variante PCF8574A). **Ne jamais prendre 0x3E**, c'est l'adresse du LCD : régler les cavaliers A0–A2 en conséquence ;
  - INT (sortie à drain ouvert, active à 0) → A2, avec la pull-up interne de l'Arduino.
- **Traitement local** :
  - **au repos**, les 4 lignes sont à 0 et les colonnes en entrée (pull-up faible du PCF8574). Un appui tire une colonne à 0, le PCF8574 baisse INT : **pas de scrutation inutile du bus** ;
  - INT déclenche une interruption *pin change* (PCINT1, broche A2), qui notifie `tHmi`, la tâche propriétaire du bus I2C logiciel ;
  - `tHmi` attend 20 ms (anti-rebond), puis **balaye** la matrice : pour chaque ligne, une écriture (cette ligne à 0, les autres à 1) et une lecture des colonnes, soit 8 petites transactions I2C (~3 ms à 50 kHz) ;
  - la touche est confirmée si le balayage donne la même touche deux fois de suite, puis la matrice revient au repos ; l'événement part dans `qEvents` ;
  - deux touches à la fois (ou plus) : balayage ambigu, ignoré et compté dans le registre DIAG ;
  - mauvais code validé par `#` : faute.

### 2.5 Relais (3 boutons Grove + 3 LED) : module de mémoire

- **Matériel** : 3 boutons Grove, chacun sous une LED (rouge, verte, bleue).
- **Le démineur voit** : à chaque étape (3 étapes), une LED s'allume.
- **Extrait de manuel** :
  > *Étape 1 : rouge → bouton sous la LED bleue ; vert → bouton du milieu ; bleu → même couleur.*
  > *Étape 2 : rouge → le bouton pressé à l'étape 1 ; vert → bouton de gauche ; bleu → bouton sous la LED rouge.*
  > *Étape 3 : …*

  Les règles renvoient aux étapes précédentes : l'expert doit suivre l'historique.
- **Traitement local** : interruptions *pin change* (PCINT2) et **anti-rebond** par horodatage dans l'ISR (exactement l'exercice de la phase 1).

### 2.6 Détonateur (interrupteur DIP 6 positions)

- **Matériel** : interrupteur DIP 6 positions, LCD, bouton du milieu pour valider.
- **Le démineur voit** : `CANAL 27` sur le LCD. Tous les interrupteurs sont à OFF (vérifié avant le démarrage).
- **Extrait de manuel** :
  > *Écrivez le canal en binaire sur 6 bits, l'interrupteur 1 étant le bit de poids fort. Si le numéro de série contient la lettre X, inversez tous les bits. Si vous avez déjà fait une faute, décalez le code d'un cran vers la droite.*
  > *Attention : basculer un interrupteur qui doit rester sur OFF déclenche une faute immédiate.*

  L'expert fait le calcul binaire et dicte « haut, bas, bas, haut… ». Les erreurs de gauche/droite et d'inversion sont fréquentes, c'est voulu.
- **Traitement local** :
  - lecture du DIP par I2C (bus logiciel, §3.2) toutes les 50 ms, avec détection de changement ;
  - un interrupteur basculé alors que son bit attendu est 0 donne une faute tout de suite. C'est une échéance **souple** (≈ 100 ms), à ne pas confondre avec celle du Périmètre ;
  - validation au bouton : compare les 6 bits au code attendu.
- **Usage hors partie (option)** : bouton 1 maintenu au démarrage → l'Arduino lit le DIP comme **configuration d'atelier**. Interrupteur 1 : mode silencieux (pas de buzzer, pour travailler en salle). Interrupteur 2 : mode banc de mesure de latence. Interrupteurs 5–6 : adresse I2C esclave (0x42 à 0x45), pour brancher plusieurs bombes sur un même RPi.

### 2.7 Finale : le fil rouge

- Le LCD affiche `COUPEZ LE FIL`, les 3 LED s'allument, la LED Bar se transforme en mèche.
- **Extrait de manuel** : *« Maintenez le bouton sous la LED bleue si vous avez 0 faute, sous la rouge sinon. Relâchez quand l'afficheur du chrono contient un 4. »* Sous la minute, l'afficheur montre les centièmes : un 4 y apparaît souvent, mais seul le démineur le voit, d'où la tension.
- Relâcher au mauvais moment ou sur le mauvais bouton : faute, et on recommence la finale.

### 2.8 Composition par niveau

| Niveau | Modules | Durée | Fautes tolérées | Seuil Périmètre | Saboteur |
|---|---|---|---|---|---|
| Recrue | Périmètre, Clavier, Relais (+ LED Bar en mode assisté) | 6 min | 3 | 10 cm | non |
| Démineur | + Fréquence, Occultation | 5 min | 2 | 15 cm | optionnel |
| Expert | + Détonateur (tous), Clavier en « code croisé » | 4 min | 1 | 20 cm | oui |

Tous ces paramètres sont réglables individuellement côté cloud (durée, nombre de modules, fautes, seuil, accélération par faute).

---

## 3. Architecture matérielle

### 3.1 Brochage (Arduino Uno + Grove Base Shield v2)

Le shield a 7 ports numériques (D2–D8), 4 analogiques (A0–A3), 4 I2C (tous reliés au bus A4/A5) et 1 UART. Un port Grove Dn donne accès à Dn (fil jaune) et Dn+1 (fil blanc). Ce qui n'a pas de port passe par des **câbles jumper**. Sur les 20 broches du Uno, **19 sont utilisées : seule A3 reste libre**.

| Broche | Module | Branchement | Pourquoi cette broche |
|---|---|---|---|
| D0 / D1 | **LED Bar** (données D0, horloge D1) | port **UART** | Seule paire libre. Coût : plus de port série dans le firmware final (voir plus bas) |
| D2 | Ultrasons | port D2 | **INT0** : interruption externe dédiée, la plus précise pour horodater l'écho |
| D3 | Buzzer | port D3 | `tone()` (Timer2) pour jouer des notes |
| D4, D5, D6 | Boutons Grove 1, 2, 3 | ports D4, D5, D6 | **PCINT20–22** (groupe PCINT2, comme dans la phase 1) : une seule ISR pour les 3 boutons |
| D7 / D8 | **Bus I2C logiciel** (SCL = D7, SDA = D8) → hub → LCD + DIP + PCF8574 du clavier | **port D7** | Les deux broches forment un port Grove : le hub s'y branche directement. Voir 3.2 |
| D9, D10, D11 | LED rouge, verte, bleue | jumpers | Pas de port Grove sur D9–D13 |
| D12 / D13 | **Afficheur 4 chiffres** (CLK D12, DIO D13) | jumpers | Dernière paire libre |
| A0 | Potentiomètre | port A0 | ADC |
| A1 | Capteur de lumière | port A1 | ADC |
| A2 | **INT du PCF8574** (clavier) | jumper | PCINT10 (groupe PCINT1) : réveil sur appui, sans scruter le bus |
| A3 | *libre* | — | Réserve |
| A4 / A5 | **I2C esclave vers le RPi** | port I2C ou jumpers vers GPIO2 (SDA) / GPIO3 (SCL) du RPi, via l'adaptateur de niveau | Seul contrôleur TWI matériel du Uno |

**Perte du port série.** La LED Bar n'a que des entrées haute impédance, elle ne gêne pas le téléversement par USB. En revanche le firmware final ne peut plus utiliser `Serial`. Solution : un drapeau de compilation `DEBUG_SERIAL` qui désactive la LED Bar pendant le développement (on la débranche), et des **mesures qui passent par le registre DIAG** de l'I2C, donc par le RPi. Si l'équipe préfère garder le port série, c'est la LED Bar qu'il faut sacrifier : c'est le composant le moins indispensable.

**Courant** : LED Bar pleine (~10 × 20 mA), afficheur, LCD et LED dépassent vite 300 mA. La LED Bar est réglée à mi-intensité (le MY9221 gère 8 bits de luminosité par segment), et l'Arduino est alimenté par un port USB capable de fournir 500 mA.

### 3.2 Le piège du LCD et du DIP sur l'I2C

Le LCD Grove 16x2 (contrôleur JHD1802, adresse 0x3E), l'interrupteur DIP 6 positions (microcontrôleur intégré, adresse 0x03) et le PCF8574 du clavier (0x20) sont des périphériques **I2C**. Or le seul contrôleur I2C du Uno doit être **esclave** du Raspberry Pi. Le faire basculer maître/esclave sur le même bus reviendrait à du multi-maître, que le contrôleur I2C du RPi gère mal.

**Solution retenue** : un **second bus I2C logiciel** (bit-banging, ~50 kHz) sur D7/D8, où l'Arduino est maître et le LCD, le DIP et le PCF8574 les esclaves. Ils se branchent sur le port Grove D7 via un **hub I2C Grove** à 4 ports : un vers l'Arduino, trois pour les périphériques (le PCF8574, qui n'est pas Grove, par un câble Grove → jumper). Au démarrage, l'Arduino **scanne le bus** et signale tout périphérique absent ou en conflit d'adresse.

- Le driver bit-bang laisse les interruptions **actives** : une préemption ralentit seulement l'horloge I2C, ce que le protocole tolère. Pas de section critique longue, donc pas d'impact sur la contrainte de 50 ms.
- Le DIP embarque un microcontrôleur qui peut **étirer l'horloge** : le driver relâche SCL puis attend qu'elle remonte vraiment avant de continuer (facile à faire en bit-bang).
- La bibliothèque Grove du DIP (`Grove_Multi_Switch`) s'appuie sur `Wire`, donc sur le bus matériel. Il faut réécrire sa lecture sur notre bus logiciel (quelques lignes, protocole à relever dans la bibliothèque).
- **Un seul propriétaire du bus** : la tâche `tHmi` fait à la fois les écritures LCD, la lecture du DIP et le balayage du clavier. Les écritures LCD sont découpées (seuls les caractères modifiés) pour qu'un appui de touche n'attende jamais plus de quelques ms. Pas de mutex, donc **pas d'inversion de priorité possible** sur ce bus. L'alternative (mutex avec héritage de priorité partagé entre `tHmi` et `tSense`) est à citer dans l'article comme choix écarté.

L'afficheur 4 chiffres (TM1637) et la LED Bar (MY9221) ne sont **pas** de l'I2C : chacun a son protocole série à 2 fils, pilotés en bit-bang sur leurs broches dédiées. Ils sont simples et rapides (~1 ms pour une mise à jour complète).

Plan B : le LCD sur le bus du RPi, piloté par la passerelle. On perd l'autonomie de l'affichage, ce qui est contraire au sujet.

### 3.3 Le bus I2C Arduino ↔ RPi

- **Câblage** : A4 (SDA) ↔ GPIO2 (broche 3 du RPi), A5 (SCL) ↔ GPIO3 (broche 5), masse commune (broche 6), avec des jumpers.
- **Niveaux** : le RPi est en 3,3 V, le Uno en 5 V. La bibliothèque Wire active les pull-ups internes vers 5 V : **danger pour le RPi**. Utiliser un **adaptateur de niveau bidirectionnel** (type BSS138, quelques euros). À défaut, désactiver les pull-ups internes après `Wire.begin()` et laisser ceux du RPi (1,8 kΩ vers 3,3 V). Ça marche en pratique mais c'est **hors spécification** (V_IH min du 328P = 3,5 V à 5 V), à signaler dans l'article.
- **Étirement d'horloge** : le contrôleur I2C du RPi gère mal le *clock stretching*, que l'esclave AVR pratique pendant `onRequest`. Parades : bus à 50 kHz (`dtparam=i2c_arm_baudrate=50000`), réponse `onRequest` **pré-calculée** (simple copie d'un buffer), CRC sur les trames pour détecter les octets corrompus.

### 3.4 Boîtier

Boîtier imprimé en 3D (valise ou bâtons de dynamite).

- **Face avant** : l'afficheur 4 chiffres en grand, la LED Bar juste dessous, le **clavier 12 touches** à côté.
- **Face supérieure** : LCD, les 3 LED au-dessus des 3 boutons Grove, l'œil ultrason, la cellule photosensible juste à côté (piège), le DIP sous un petit volet « DÉTONATEUR ».
- **Côté** : le cadran (potentiomètre) avec graduations imprimées.
- **Intérieur** : Arduino, et si possible le **RPi et une batterie USB**. La bombe devient autonome en Wi-Fi, plus spectaculaire en démo, avec un câble I2C court.

---

## 4. Architecture logicielle Arduino (FreeRTOS)

Bibliothèque `Arduino_FreeRTOS` (feilipu). Son tick par défaut vient du **watchdog (~15 ms)**, ce qui laisse les Timer1 et Timer2 libres.

### 4.1 Répartition des timers

| Timer | Usage | Justification |
|---|---|---|
| Timer0 | `millis()` / `micros()` (Arduino) | Inchangé |
| Timer1 | CTC, période 20 ms → **déclenche le ping ultrason** ; prescaler 8 → **0,5 µs** de résolution pour horodater l'écho | Échantillonnage du capteur critique indépendant du tick FreeRTOS |
| Timer2 | `tone()` du buzzer (notes, sirène, mélodie) | Libre depuis l'abandon de l'IR. Son interruption (une par demi-période du son) ajoute un peu de gigue : à mesurer (§6.3) |
| WDT | Tick FreeRTOS | Valeur par défaut de la bibliothèque |

### 4.2 Interruptions

| ISR | Source | Travail (minimal) |
|---|---|---|
| `TIMER1_COMPA` | toutes les 20 ms | si la ligne écho est revenue à 0 : impulsion de 10 µs, passage en entrée, arme INT0 |
| `INT0` | front de l'écho | horodate (TCNT1) ; sur le front descendant : calcule la distance, si < D deux fois de suite → `vTaskNotifyGiveFromISR(tSafety)` + `portYIELD_FROM_ISR()` |
| `PCINT2` | boutons Grove | lit `PIND`, anti-rebond 30 ms par horodatage, envoie l'événement dans `qEvents` |
| `TIMER2_COMPA` | `tone()` | bascule la broche du buzzer |
| `PCINT1` | INT du PCF8574 (A2) | si INT est à 0 : notifie `tHmi` (une touche vient d'être pressée) |
| `TWI` (Wire) | RPi | `onReceive` : copie dans un buffer + notifie `tComm`. `onRequest` : renvoie un buffer **déjà prêt** |

L'ISR du clavier ne touche pas au bus I2C (un balayage prend ~3 ms, beaucoup trop pour une ISR) : elle se contente de réveiller la tâche qui possède le bus.

### 4.3 Tâches

| Tâche | Prio | Déclenchement | Rôle |
|---|---|---|---|
| `tSafety` | **3** (max) | notification de l'ISR ultrason | Allume tout de suite la LED rouge (écriture directe sur le port) et lance le son de faute, horodate la latence, pousse `FAULT` dans `qEvents` |
| `tGame` | 2 | `qEvents` + timeout 100 ms | Machine à états de la partie, logique des modules, chrono (avec facteur de vitesse), fautes, explosion |
| `tSense` | 2 | périodique 20 ms | ADC cadran et lumière : filtrage, hystérésis, calibration, occultations → `qEvents` |
| `tComm` | 1 | notification de l'ISR TWI | Décode les commandes, vérifie le CRC de la configuration, construit l'instantané d'état pour le RPi |
| `tHmi` | 1 | périodique 50 ms **ou** notification de l'ISR clavier | Propriétaire du bus I2C logiciel : LCD (lent, préemptible), **lecture du DIP** et **balayage du clavier** (touches et changements → `qEvents`). Afficheur 4 chiffres (chrono, 20 Hz pour les centièmes), LED Bar, motifs des LED et du buzzer |
| *idle* | 0 | — | Hook : compteur de charge CPU, puis mise en veille (`SLEEP_MODE_IDLE`) |

**Pourquoi ces priorités** : seule la chaîne de sécurité a une échéance dure, elle est donc au-dessus de tout. Le jeu et l'acquisition passent avant l'IHM : un LCD qui se rafraîchit 30 ms en retard ne se voit pas, alors qu'une touche perdue, si. `tComm` est bas parce que le RPi interroge (il réessaie) et que les réponses I2C sont déjà prêtes en ISR.

### 4.4 Communication entre tâches

- **Notification de tâche** (ISR → `tSafety`) : la primitive FreeRTOS la plus rapide, sans objet noyau.
- **File `qEvents`** (ISR et tâches → `tGame`) : boutons, touches du clavier, occultations, DIP, fautes, commandes. Un **seul écrivain** pour l'état du jeu (`tGame`), donc pas de course sur le compteur de fautes.
- **Mutex `mState`** (avec héritage de priorité) : protège la lecture de l'état par `tHmi` et `tComm`.
- **Instantané I2C en double tampon** : `tComm` écrit le tampon inactif, puis échange l'index dans un `ATOMIC_BLOCK` (cf. note de bas de page du sujet sur `util/atomic.h`). L'ISR TWI lit toujours un tampon cohérent, sans verrou.
- **FIFO d'événements vers le RPi** : tampon circulaire de 16 événements avec numéro de séquence. Le RPi détecte les pertes.
- **Priorité d'actionneur** : `tSafety` écrit directement LED rouge et buzzer et pose un `alarmUntil`. `tHmi` ne touche pas à ces sorties avant cette échéance.

### 4.5 Budget mémoire (2 Ko de RAM, c'est la vraie contrainte du Uno)

| Poste | Octets (estimation) |
|---|---|
| 5 tâches (TCB ≈ 40 o + pile 128–192 o) | ~1000 |
| Wire (5 tampons de 32 o + état) | ~170 |
| Configuration de partie | ~48 |
| FIFO d'événements (16 × 6 o) | 96 |
| Instantanés I2C (2 × 16 o) | 32 |
| Tampon LCD (32 car.) + glyphes | 40 |
| Afficheur 4 chiffres, LED Bar, DIP, clavier (saisie en cours) | ~30 |
| Variables globales, Arduino core (sans `Serial` : −~150 o de tampons) | ~120 |
| **Total** | **~1540 / 2048** |

- **Allocation statique** (`configSUPPORT_STATIC_ALLOCATION`, `xTaskCreateStatic`) : l'occupation est connue à la compilation, pas de fragmentation.
- Mesure des piles avec `uxTaskGetStackHighWaterMark` (remontées dans le registre DIAG).
- Si ça déborde : fusionner `tSense` dans `tGame`, ou remplacer Wire par un driver TWI esclave maison (registres directement, tampons de 16 o).

---

## 5. Machine à états

```
         CONFIG (CRC ok)   DIP tout OFF + 3 boutons 1 s     tous les modules OK
 IDLE ─────────────────> ARMED ─────────────────> RUNNING ─────────────────────> FINALE
  ^                        │                     │    │                            │
  │       RESET            │ ABORT               │    │ fautes > max ou t = 0      │ bon relâché
  └────────────────────────┴─────────────────────┘    v                            v
                                                   EXPLODED                     DEFUSED
```

Dans `RUNNING`, sous-état `module_courant` (ordre donné par la configuration). `Périmètre` est actif de `RUNNING` à `FINALE` inclus.

---

## 6. La contrainte temps réel dure : faute détectée et signalée en < 50 ms

### 6.1 Chaîne

```
intrusion ─► ping Timer1 (≤ 20 ms) ─► écho (≤ 1,2 ms pour 20 cm) ─► ISR INT0
         ─► 2e échantillon (+20 ms) ─► notification + yield ─► tSafety (prio max)
         ─► PORTB |= LED_ROUGE ; son de faute
```

### 6.2 Budget pire cas

| Étape | Pire cas |
|---|---|
| Attente du prochain ping | 20 ms |
| Temps de vol pour D = 20 cm | 1,2 ms |
| Confirmation sur un 2e échantillon | 20 ms + 1,2 ms |
| ISR + `portYIELD_FROM_ISR` + changement de contexte | ~50 µs (à mesurer) |
| Plus longue section masquée (ISR TWI, ISR `tone()`, sections critiques FreeRTOS) | ~100 µs (à mesurer) |
| Écriture GPIO dans `tSafety` | < 5 µs |
| **Total** | **≈ 42,6 ms < 50 ms** |

Le chemin critique **ne dépend pas du tick** FreeRTOS : la notification depuis l'ISR préempte immédiatement. Le tick imprécis du watchdog ne pèse donc pas sur l'échéance. C'est un argument à mettre en avant à l'oral.

**Hypothèse environnementale à écrire dans l'article** : le Grove Ultrasonic n'a qu'une broche (déclenchement et écho). On ne peut pas relancer un ping tant que l'écho précédent n'est pas retombé. Avec un plafond à 2,5 m, l'écho le plus long dure ~15 ms, ce qui reste compatible avec une période de 20 ms. Dans un grand hall, la période s'allonge et l'échéance peut sauter. La documentation conseille ≥ 60 ms entre deux mesures : nous descendons à 20 ms en connaissance de cause et **mesurons le taux de faux positifs**. Bonne matière pour l'analyse critique.

**Plan B** si l'ultrason ne tient pas l'échéance :

- confirmation sur **un seul** échantillon (budget ≈ 21 ms), au prix de plus de faux positifs ;
- ou contrainte dure portée par la **cellule photosensible** (« ne jamais masquer la cellule hors du module Occultation »), échantillonnée à 1 kHz par le Timer1 : détection en quelques ms. L'ultrason devient alors une contrainte souple.

### 6.3 Mesure (pour l'article)

1. **Instrumentation interne** : `TCNT1` lu au front d'écho qui valide la faute, puis au moment où `tSafety` écrit la LED. Min, moyenne, max et compteur de dépassements dans le registre DIAG, lu par le RPi et envoyé au cloud.
2. **Mesure externe** avec un **analyseur logique** USB (clone Saleae 8 voies à ~10 €, fortement recommandé) : voie 1 sur D2 (écho), voie 2 sur D9 (LED rouge). Histogramme sur quelques milliers de fautes provoquées.
3. **Scénarios de charge** : (a) repos ; (b) LCD qui rafraîchit en continu ; (c) RPi qui interroge à 100 Hz ; (d) buzzer qui joue des notes (ISR du Timer2) + frappe rapide au clavier et aux boutons ; (e) tout à la fois + sabotages.
4. **Comparaison avec une version *superloop* naïve** (`loop()` avec `pulseIn()` bloquant + LCD + Wire) : montrer qu'elle **rate l'échéance** sous charge. C'est la justification chiffrée de l'usage du RTOS demandée pour l'oral.

---

## 7. Protocole I2C (Arduino esclave, adresse 0x42)

Le RPi écrit le numéro de registre puis lit N octets (start répété, `I2C_RDWR`). La bibliothèque Wire limite une trame à **32 octets**, d'où la configuration envoyée en pages.

| Reg | Nom | Accès | Taille | Contenu |
|---|---|---|---|---|
| `0x00` | STATUS | R | 10 | état, temps restant (uint16, 1/10 s), fautes, fautes max, masque des modules résolus, module courant, facteur de vitesse (%), position du DIP (6 bits), CRC-8 |
| `0x08` | EVT_COUNT | R | 1 | événements en attente |
| `0x09` | EVT_POP | R | 7 | séquence, type, module, données (2), t depuis le départ (2), puis l'événement est retiré |
| `0x10` | DIAG | R | 12 | latence RT max et moyenne (µs), dépassements, charge CPU %, pile min, erreurs I2C (deux bus), balayages clavier rejetés, uptime |
| `0x20` | CMD | W | 1–4 | `01` ARM, `02` START (démarrage à distance), `03` ABORT, `04` RESET, `10` SABOTAGE_ACCEL(facteur, durée), `11` SABOTAGE_BROUILLAGE(durée), `12` FAUSSE_ALARME, `20` PING(nonce) |
| `0x40`–`0x41` | CONFIG p0–p1 | W | ≤ 30 chacune | blob de configuration, CRC-8 en dernière position ; ARM refusé si CRC faux |

**Contenu de la configuration** (~48 o) : durée, fautes max, ordre des modules, seuil D, nombre d'échantillons de confirmation, accélération par faute, n° de série (6 car.), mot de Fréquence (8 car.) et cran attendu, N éclairs et nombre d'occultations attendu, numéro de lot et code attendu du Clavier (4 chiffres), réponses des 3 étapes du Relais (stimulus + bouton), canal affiché et code attendu du Détonateur (6 bits), bouton et chiffre de la finale.

**Types d'événements** : `FAULT(module, cause)`, `MODULE_SOLVED`, `STATE_CHANGE`, `SABOTAGE_APPLIED`, `PONG(nonce)`.

---

## 8. Passerelle Raspberry Pi (fog / edge)

### 8.1 Rôle

- **Maître I2C** en interrogation uniquement (conforme au sujet) : STATUS à 20 Hz, vidage de la FIFO quand `EVT_COUNT > 0`, DIAG toutes les 5 s.
- **Traduction** des registres en JSON/MQTT, et des commandes MQTT en écritures de registres (pagination et CRC de la configuration).
- **Filtrage** : l'état n'est publié **qu'en cas de changement** ou à 1 Hz (au lieu de 20 Hz), les événements tout de suite, les données de diagnostic agrégées. On mesure le gain de bande passante.
- **Horodatage** : l'Arduino donne un temps relatif au départ ; le RPi (synchronisé NTP) le convertit en temps absolu.
- **Chien de garde** : si STATUS ne répond plus ou si l'uptime ne bouge plus pendant 1 s, publication de `BOMB_OFFLINE`.
- **Stockage et renvoi** : si le cloud est injoignable, événements et résultats vont dans une file locale (SQLite ou fichier en ajout seul) et repartent à la reconnexion. La bombe, elle, continue de jouer.
- **Effet d'explosion** (option) : le RPi joue un son d'explosion sur un haut-parleur (prise jack), puisqu'il reçoit l'événement `EXPLODED`.

### 8.2 Topics MQTT

| Topic | Sens | QoS | Contenu |
|---|---|---|---|
| `bomb/{id}/state` | ↑ | 0, *retained* | état, temps restant, fautes, modules |
| `bomb/{id}/event` | ↑ | 1 | événements numérotés |
| `bomb/{id}/diag` | ↑ | 0 | latence RT, CPU, piles, erreurs I2C |
| `bomb/{id}/cmd/config` | ↓ | 1 | blob de configuration |
| `bomb/{id}/cmd/{arm,start,abort,reset}` | ↓ | 1 | |
| `bomb/{id}/cmd/sabotage` | ↓ | 1 | type, durée, horodatage d'envoi |

### 8.3 Optimisation (exigence de la phase 3)

Comparer des déploiements, mesures à l'appui :

| Variante | Description |
|---|---|
| A. Référence | Raspberry Pi OS complet + script Python lancé à la main |
| B. Allégée | Raspberry Pi OS **Lite**, services inutiles désactivés (bluetooth, avahi, cups…), démon Python sous **systemd** |
| C. Optimisée | Démon **binaire statique** (Go, Rust ou C) sous systemd avec durcissement (`MemoryMax`, `CPUQuota`, `ProtectSystem`), ou dans un conteneur `FROM scratch` de quelques Mo |
| D. Bonus | Image **Buildroot** minimale qui démarre directement sur le démon |

Métriques : temps de démarrage jusqu'à « prêt », RAM (RSS), CPU, taille de l'image, gigue de la période d'interrogation, écritures sur la carte SD.

---

## 9. Cloud

Peut être réel (petit VPS) ou simulé sur un portable (`docker compose`), comme le sujet l'autorise.

- **Broker** Mosquitto.
- **API** (FastAPI ou Node) + **SQLite**.
- **Pages web** :
  - `/lobby` : création d'équipe, choix du niveau ou réglage fin des paramètres ;
  - `/manuel/{partie}` : le manuel pour l'expert, pensé pour le mobile, **sans la solution** ;
  - `/saboteur/{partie}` : 3 boutons de sabotage, budget de 3 actions et 30 s de recharge ;
  - `/live` : vue spectateurs (chrono, fautes, modules) pour la soutenance ;
  - `/classement`.
- **Génération** : graine 32 bits → PRNG déterministe (mulberry32 ou xorshift) → n° de série, mots, numéro de lot, tables, ordre des modules, stimuli, solution. Le manuel est rendu en HTML ; la configuration est sérialisée au format binaire de la bombe.
- **Test automatique du générateur** : un **solveur** lit le manuel généré et la configuration, et vérifie que les règles donnent bien la solution envoyée à la bombe, sur 10 000 graines. Ça évite des parties impossibles le jour de la démo.
- **Score** : `temps restant × multiplicateur de niveau − 15 s × fautes`, avec bonus si un saboteur était actif.
- **Mesure de bout en bout** : la commande `PING(nonce)` fait l'aller-retour cloud → RPi → Arduino → RPi → cloud. Latence d'un sabotage jusqu'à son effet sur la bombe, et d'une faute jusqu'à son affichage sur `/live`.

---

## 10. Expériences à mettre dans l'article

| # | Question | Mesure |
|---|---|---|
| E1 | L'échéance de 50 ms est-elle tenue sous charge ? | Histogramme de latence par scénario (§6.3), RTOS contre *superloop* |
| E2 | Quel coût pour le RTOS sur un Uno ? | Charge CPU (idle hook), piles, RAM totale, taille du binaire |
| E3 | Quel compromis pour l'interrogation I2C ? | Fréquence d'interrogation contre latence des événements contre CPU (deux côtés) ; erreurs I2C avec et sans adaptateur de niveau, à 50 et 100 kHz |
| E4 | La passerelle est-elle légère ? | Variantes A–D (§8.3) |
| E5 | Quelle latence de bout en bout ? | Événement → `/live`, sabotage → effet sur la bombe |
| E6 | Le système est-il robuste ? | Couper le Wi-Fi pendant la partie (la bombe continue, les événements repartent sans perte) ; débrancher le RPi (l'Arduino reste autonome) |
| E7 | Les capteurs sont-ils fiables ? | Faux positifs du Périmètre selon la période et le seuil ; touches perdues ou doublées au clavier en frappe rapide ; temps entre l'appui et l'événement |

---

## 11. Risques et plans B

| Risque | Probabilité | Plan B |
|---|---|---|
| Échos parasites de l'ultrason, échéance non tenue | moyenne | Confirmation sur un seul échantillon, ou contrainte dure déplacée sur la cellule photosensible (§6.2) |
| Pas de module PCF8574 disponible | moyenne | Le keypad en direct demande 7 broches : impossible. Plan B : 7 résistances soudées sur les fils du keypad pour le lire sur la seule entrée analogique A2 (montage classique « keypad sur une broche ») |
| Conflit d'adresse I2C (PCF8574A et LCD à 0x3E) | faible | Régler les cavaliers A0–A2 ; scan du bus au démarrage |
| Touches perdues si le LCD occupe le bus | faible | Écritures LCD découpées ; réveil du clavier par INT ; mesure E7 |
| RAM du Uno saturée | moyenne | Fusion de tâches, driver TWI maison, glyphes en PROGMEM |
| Octets corrompus sur l'I2C (étirement d'horloge du RPi) | élevée sans précaution | 50 kHz, réponses pré-calculées, CRC + nouvelle tentative côté RPi |
| 5 V sur l'I2C du RPi | — | Adaptateur de niveau (à demander ou acheter) |
| Plus de broches | certaine | Brochage du §3.1 : 19 broches sur 20, A3 en réserve. Toute nouvelle idée passe par ce tableau |
| Plus de port série pour déboguer | certaine | Drapeau `DEBUG_SERIAL` (LED Bar débranchée en développement), mesures par le registre DIAG ; sinon renoncer à la LED Bar |
| Protocole I2C du DIP mal documenté, ou étirement d'horloge | moyenne | Relever le protocole dans `Grove_Multi_Switch` dès la S2 ; driver bit-bang qui attend SCL |
| Consommation trop forte (LED Bar + afficheur + LCD) | faible | LED Bar à mi-intensité, alimentation USB 500 mA |
| Cas d'usage refusé en semaine 2 | faible | Préparer une fiche d'une page (concept + liste des capteurs et actionneurs + contrainte de 50 ms) |

---

## 12. Planning (calé sur les phases du sujet)

| Semaine | Phase | Livrables |
|---|---|---|
| S1 | 1 : bare-metal | Boutons PCINT2 + anti-rebond, Timer1 CTC, LED, buzzer avec `tone()`. **Commande du PCF8574** (ou plan B) : premier test du keypad et de son balayage |
| S2 | 2 : drivers | **Validation du cas d'usage par l'équipe enseignante.** Drivers ultrason (INT0 + Timer1), ADC filtré (cadran, lumière), I2C logiciel pour le LCD, le DIP et le clavier (PCF8574 + INT), afficheur 4 chiffres (TM1637), LED Bar (MY9221) |
| S3 | 2 : FreeRTOS | Migration vers les tâches, chaîne de sécurité, **première mesure de latence**, budget mémoire |
| S4 | 2 : I2C | Esclave + registres sur l'Arduino, maître sur le RPi, adaptateur de niveau, CRC |
| S5 | 3 : passerelle | Démon de passerelle, MQTT, filtrage, stockage et renvoi ; cloud : générateur et manuel |
| S6 | 3 : passerelle | Variantes d'optimisation A–D et mesures ; cloud : saboteur, classement, `/live` |
| S7 | 4 : finalisation | Boîtier imprimé, intégration, campagnes de mesure E1–E7, article |
| S8 | Soutenance | Démo en direct (l'expert sur son téléphone, un saboteur dans le public) |

Rendu de l'article : **avant le 17 janvier 2027 à 22h00**, 10 pages maximum.

**Répartition suggérée** (à adapter à la taille de l'équipe) :

1. Drivers bare-metal, clavier et boîtier ;
2. FreeRTOS, chaîne temps réel, campagnes de mesure ;
3. I2C et passerelle RPi (optimisation) ;
4. Cloud : générateur de manuel, pages web, classement.

---

## 13. Matériel à ajouter (hors kit)

| Élément | Nécessité | Usage |
|---|---|---|
| Adaptateur de niveau I2C 3,3 V ↔ 5 V | **Fortement recommandé** | Protéger le RPi |
| Câbles Grove → jumper | Disponibles | LED sur D9–D11, afficheur sur D12/D13, PCF8574 vers le hub et INT vers A2, I2C vers le RPi |
| Hub I2C Grove 4 ports (ou répartiteur sur plaque d'essai) | **Nécessaire** | LCD, DIP et PCF8574 sur le port D7 |
| Module PCF8574 (extension I2C 8 E/S, ~2 €) | **Nécessaire** | Brancher le keypad 3×4 (7 fils) sans broche de l'Arduino. Les « backpacks » I2C de LCD en sont aussi, mais leurs broches sont souvent soudées à un connecteur 16 points : prendre un module nu |
| Analyseur logique USB 8 voies | Recommandé | Mesure externe de la latence (E1) |
| Impression 3D (boîtier avec découpe pour le keypad) | Nécessaire | |
| Batterie USB | Recommandé | Bombe autonome en Wi-Fi |
| Haut-parleur jack | Option | Son d'explosion côté RPi |
| Accéléromètre (si dispo à l'Ubiquarium) | Option | Rétablir le module « Stabilisateur » d'origine (les accéléromètres Grove sont en I2C : sur le hub du bus logiciel) |
