# Earth Phaser — by NACHE

Phaser stéréo (VST3 / Standalone, Windows) calqué sur l'algorithme **Phaser-DDL** de l'Ensoniq DP/4 :
phaser stéréo à **12 pôles** (12 allpass par canal) + delay ping-pong qui **rebouche dans l'entrée du phaser**.
Construit avec JUCE 8 (téléchargé automatiquement par CMake).

## Contrôles
Coins : **RATE**, **WIDTH** (excursion des notchs), **CENTER** (centre du balayage, bipolaire), **FEEDBACK** (résonance, bipolaire : le signe change la polarité)
Côtés : **NOTCH DEPTH**, **L/R LFO** (déphasage du LFO gauche/droite, 0–180°), **DELAY** (temps, R = L/2 comme sur le DP/4), **DLY FEEDBACK**
Globe / anneau : **Dry/Wet** · Interrupteur on/off · VU-mètre · menu de presets (25 d'usine + presets utilisateur) · double-clic sur le titre : menu caché.
Les valeurs sont affichées sur l'échelle 0–99 du DP/4.

## Compiler sur GitHub (Windows)
1. Créez un dépôt GitHub vide, puis dans le dossier décompressé :
```
git init
git add .
git commit -m "Earth Phaser 1.0"
git branch -M main
git remote add origin https://github.com/TON_PSEUDO/EarthPhaser.git
git push -u origin main
```
2. Onglet **Actions** → *Build Earth Phaser (Windows)* (ou *Run workflow*).
3. Téléchargez l'artefact `EarthPhaser-Windows` en bas de l'exécution.
4. Copiez `Earth Phaser.vst3` dans `C:\Program Files\Common Files\VST3` et relancez le scan de votre DAW.

## Personnalisation
- Police « Thriller » : placez `Thriller.ttf` dans `Resources/`.
- Titre / version / date / lien : constantes en haut de `Source/PluginEditor.cpp`.
- Presets d'usine : `Source/Presets.h`.
