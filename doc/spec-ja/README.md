# FHE-Table-Search Basic Specification

# Prerequisites
* Linux or MacOSX
* pip

# How to build
1. Install sphinx and some modules
```
$ pip install sphinx commonmark recommonmark sphinx_rtd_theme sphinx_fontawesome
```
2. Install tex environment (to generate pdf)
  * for Linux (install TeX Live)
  ```
  $ sudo apt -y install \
       texlive-latex-recommended \
       texlive-latex-extra \
       texlive-fonts-recommended \
       texlive-fonts-extra \
       texlive-lang-japanese \
       texlive-lang-cjk \
       latexmk
  ```
  * for MacOSX (install MacTeX)
  ```
  $ brew cask install mactex-no-gui
  $ sudo tlmgr update --self --all
  ```
3. Build specification
```
$ make html      # to build HTML
$ make latexpdf  # to build PDF
```
