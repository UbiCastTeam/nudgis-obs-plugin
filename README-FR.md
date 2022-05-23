# Plugin Nudgis OBS Studio

## Description

Le plugin Nudgis OBS Studio, permet d' interfacer facilement le logiciel OBS (https://obsproject.com) avec la plateforme Nudgis de la société Ubicast (https://www.ubicast.eu/fr/solutions/diffusion/ ).

Les fonctionnalités du plugin couvre les actions suivantes:
- réalisation d'un Live
- téléchargement d'un enregistrement

Le plugin est disponible pour les plateformes suivantes:
- Windows 7/8/10/11 32/64 bits
- Mac OS
- Ubuntu 20.04 64 bits

## Installation

Le plugin est disponible sous forme de binaire directement déployable sur la plateforme souhaitée.  
Les binaires de déploiement sont générés par la CI github/actions (Artifacts).  
Les binaires de déploiement sont accessible depuis la rubrique **Actions** du dépôt du projet (https://github.com/UbiCastTeam/nudgis-obs-plugin/actions ).  
La rubrique **Actions** référence l'ensemble des builds qui ont été effectués.  
La sélection du dernier build permet d'accéder au listing de l'ensemble des Artifacts disponibles.  
Le nom de fichier d'un Artifacts respecte le format suivant:

\<nom_du_plugin>\<plateforme>\_\<version_obs_compatible>_\<version_du_plugin>  
Par exemple
<pre>
nudgis-obs-plugin-linux-ubuntu-20.04-64_27.2.4_1.0.0
</pre>

Ci dessous le listing du nom des Artifacts, contenant les binaire de déploiement pour chaque plateforme pour la version 1.0.0 du plugin Nudgis OBS Studio:

| Plateforme | Nom de l'Artifact                                    | Remarque                                                                                                                                               |
| ---------- | ---------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Linux      | nudgis-obs-plugin-linux-ubuntu-20.04-64_27.2.4_1.0.0 | package d'installation au format .deb                                                                                                                  |
| Mac OS     | nudgis-obs-plugin-macos-x86_64_27.2.4_1.0.0          | Actuellement utilisé pour les plateforme Intel et M1 (la version d'OBS M1 fonctionne sous [Rosetta](https://en.wikipedia.org/wiki/Rosetta_(software))) |
| Mac OS     | nudgis-obs-plugin-macos-arm64_27.2.4_1.0.0           | Non utilisé actuellement - en prévision des version d'OBS native M1                                                                                    |
| Windows    | nudgis-obs-plugin-windows_27.2.4_1.0.0               |                                                                                                                                                        |

### Exemple d'installation - Ubuntu-20.04

#### Installation d'OBS

<pre>
sudo apt install software-properties-common
sudo add-apt-repository ppa:obsproject/obs-studio
sudo apt update
sudo apt install ffmpeg obs-studio
</pre>

#### Installation du plugin Nudgis OBS Studio

Téléchargement de l'Artifact **nudgis-obs-plugin-linux-ubuntu-20.04-64_27.2.4_1.0.0** depuis la rubrique **Actions** du dépôt privé https://github.com/UbiCastTeam dans le dossier courant.

<pre>
unzip ./nudgis-obs-plugin-linux-ubuntu-20.04-64_27.2.4_1.0.0.zip
sudo apt install ./nudgis-obs-plugin-1.0.0-Linux.deb
</pre>


### Note concernant l'installation sur Mac OS

Le script d'installation doit être lancé en effectuant un ```clic droit Ouvrir``` et ```clic Ouvrir``` afin d'autoriser les droits d'exécutions.

A la suite de l'installation un script de désinstallation est disponible à l'URI suivante:
<pre>
/Applications/OBS.app/Contents/Resources/data/obs-plugins/nudgis-obs-plugin/nudgis-obs-plugin-uninstall.command
</pre>

## Déploiement environnement de développement (Linux - ubuntu 20.04)

Ce paragraphe décrit les étapes à réaliser permettant de déployer l'envrionnement de développement dans le but de pouvoir compiler une nouvelle version du plugin et l'exécuter en mode debug dans OBS.

### Récupération des sources OBS - compilation et installation en mode debug

Activation des dépôts deb-src dans /etc/apt/sources.list
<pre>
sudo perl -pi -0e 's/^(deb .*\n)# (deb-src)/$1$2/gm' /etc/apt/sources.list
sudo apt update
</pre>

Récupération des sources, configuration
<pre>
sudo apt build-dep obs-studio
sudo apt install git wget libwayland-dev libxkbcommon-dev libxcb-composite0-dev libpci-dev qtbase5-private-dev
git clone https://github.com/obsproject/obs-studio.git
cd obs-studio
git checkout 27.2.4 -b 27.2.4
git submodule init
git submodule update
export CI_LINUX_CEF_VERSION=$(cat .github/workflows/main.yml | sed -En "s/[ ]+LINUX_CEF_BUILD_VERSION: '([0-9]+)'/\1/p")
wget https://cdn-fastly.obsproject.com/downloads/cef_binary_${CI_LINUX_CEF_VERSION}_linux64.tar.bz2
tar -xvaf cef_binary_${CI_LINUX_CEF_VERSION}_linux64.tar.bz2
cmake -B build . -DCMAKE_BUILD_TYPE=Debug -DENABLE_PIPEWIRE=FALSE -DCEF_ROOT_DIR=${PWD}/cef_binary_${CI_LINUX_CEF_VERSION}_linux64 -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g3' -DCMAKE_C_FLAGS_DEBUG='-O0 -g3'
</pre>

Compilation et installation
<pre>
cmake --build build
sudo cmake --install build
sudo ldconfig
</pre>

### Récupération des sources du plugin Nudgis OBS Studio - compilation et installation en mode debug

<pre>
git clone git@github.com:UbiCastTeam/nudgis-obs-plugin.git
cd nudgis-obs-plugin
cmake -B build . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g3' -DCMAKE_C_FLAGS_DEBUG='-O0 -g3'
cmake --build build
sudo cmake --install build
</pre>

### Démarrage d'une session de debug gdb

Installation de gdb
<pre>
sudo apt install gdb
</pre>

Lancement d'une session de debug - avec un point d'arrêt sur la fonction d'initialisation du plugin
<pre>
gdb -ex 'set breakpoint pending on' -ex 'b src/nudgis-plugin.cpp:obs_module_load' -ex r obs
</pre>

Lancement d'une session de debug
<pre>
gdb obs
</pre>
