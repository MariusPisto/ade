# Projet bombe à désamorcer : contexte matériel

Projet du cours *OS embarqués pour l'Edge Computing* (SI5/M2, Université Côte d'Azur, 2026-2027).
Consignes : `edge_rtos_project2026.pdf`. Conception détaillée : `CONCEPTION.md`. Inventaire : `components.txt`.
L'équipe travaille en français : documents et échanges en français.

## Matériel : règles à respecter

- **Seul le matériel de `components.txt` est disponible.** Ne pas concevoir autour de capteurs absents (pas d'IMU, NFC, capteur de couleur, de bruit, tactile, anneau LED, 433 MHz…). Si un composant est ajouté ou retiré, mettre à jour `components.txt` **et** `CONCEPTION.md` (tableaux du §0, brochage du §3.1, matériel du §13).
- **Arduino Uno** (ATmega328P, 2 Ko de RAM, 20 broches : D0–D13, A0–A5) avec **Grove Base Shield v2**.
- **Ports du shield** : 7 numériques (D2 à D8), 4 analogiques (A0 à A3), 1 UART (D0/D1), 4 I2C (tous reliés au même bus A4/A5).
  - Un port Grove Dn donne accès à **Dn (fil jaune) et Dn+1 (fil blanc)**. Idem pour An.
  - Un module à 1 broche n'utilise que le fil jaune. Un module à 2 broches (afficheur 4 chiffres, LED Bar, bus I2C) utilise les deux : il bloque le port suivant.
  - Les broches D9 à D13 n'ont pas de port Grove.
- **Tous les modules Grove se branchent sur le shield.** L'équipe a des **câbles jumper** (Grove ↔ broches de l'Arduino, et vers les GPIO du Raspberry Pi) pour tout ce qui n'a pas de port.
- **Le clavier n'est pas un module Grove** : keypad 3×4 tout fait (0–9, `*`, `#`), matrice à 7 fils. Pas assez de broches pour le brancher en direct : il passe par un **module PCF8574** (extension I2C) sur le bus I2C logiciel, et la sortie INT du PCF8574 arrive sur A2.
- **I2C** :
  - le bus matériel (A4/A5) est **réservé au Raspberry Pi** : l'Arduino y est **esclave** (adresse 0x42), le RPi est maître (GPIO2 = SDA, GPIO3 = SCL, via un adaptateur de niveau 3,3 V ↔ 5 V) ;
  - le LCD 16x2 (0x3E), l'interrupteur DIP (0x03) et le PCF8574 du clavier (0x20, ou 0x38–0x3F pour un PCF8574A : **jamais 0x3E**) sont sur un **second bus I2C logiciel** (bit-bang) dont l'Arduino est maître, sur le port Grove D7 (SCL = D7, SDA = D8) via un hub I2C.
- **Les 20 broches sont comptées** : il ne reste que A3 de libre. Toute nouvelle idée doit passer par le tableau de brochage de `CONCEPTION.md` §3.1.
- **Émetteur et récepteur infrarouges** : dans le kit mais **non utilisés** (l'émetteur exige D3 et le Timer2, pris par le buzzer, et il n'y a plus de broches).
- **Timers** : Timer0 = `millis()`, Timer1 = ping ultrason (20 ms), Timer2 = `tone()` du buzzer, watchdog = tick FreeRTOS. Ne pas réaffecter un timer sans mettre à jour le §4.1.

## Contraintes du sujet

- Arduino = smart sensor/actuator **autonome** sous **FreeRTOS** ; RPi = passerelle edge, maître I2C en **interrogation** uniquement, relais MQTT/HTTP vers un cloud (réel ou simulé).
- Contrainte temps réel dure du projet : une faute du module Périmètre (ultrasons) est détectée et signalée en **moins de 50 ms**, mesurée et rapportée dans l'article.
- Validation du cas d'usage en semaine 2. Rendu : article PDF de 10 pages max, avant le **17 janvier 2027 à 22h00**.
