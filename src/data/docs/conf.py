#
# src/data/docs/conf.py
#
import os
import sys

#
# package version from version.txt file
#
def read_version() -> str:
    """
    Get package version from version.txt file
    """
    file = '../../version.txt'
    if os.path.exists(file):
        with open(file, 'r') as fob:
            proj_vers = fob.readlines()[0]
    else:
        proj_vers = '0.1.0-unknown'
    return proj_vers

#
# Project
#

project = "sd-boot"
copyright = '2026-presennt, Gene C'
author = 'Gene C'
release = read_version()

extensions = []
latex_engine = 'xelatex'
latex_use_xindy = True

latex_elements = {
    'papersize': 'letterpaper',
    'pointsize': '11pt',

    'preamble': r'''
    \usepackage{parskip}
    \usepackage{fontspec}

    % Fix the 11pt headheight layout warnings
    \setlength{\headheight}{14pt}
    \addtolength{\topmargin}{-2pt}

    % Strip vertical spaces between items
    \usepackage{enumitem}
    \setlist[itemize]{
        noitemsep, 
        topsep=6pt,             % \parskip,       % 0pt, 
        parsep=0pt, 
        partopsep=0pt,
        after=\vspace{0pt}
        }
    \setlist[enumerate]{
        noitemsep, 
        topsep=6pt,             % 0pt, 
        parsep=0pt, 
        partopsep=0pt,
        after=\vspace{0pt}
        }

    \usepackage{newunicodechar}
    \newunicodechar{␣}{\textvisiblespace}
    \tracinglostchars=0

    % Body sans serif 
    % \setmainfont{TeX Gyre Heros}[
    % \setmainfont{IBM Plex Sans}[
    \setmainfont{Source Sans 3}[
        Ligatures=TeX,
        Scale=0.92
    ]

    % Section Headers (Modern Helvetica equivalent)
    % \setsansfont{TeX Gyre Heros}[
    % \setsansfont{IBM Plex Sans}[
    \setsansfont{Source Sans 3}[
        Ligatures=TeX,
        Scale=0.92
    ]

    % Verbatim/Inline (Monospace Fira)
    \setmonofont{Fira Mono}[
        Scale=0.88
    ]
    ''',
}

latex_documents = [
    (
        'index',
        'sd-boot.tex',
        'sd-boot Documentation ',
        'Gene C',
        'manual'
    ),
]

# ==========================================
# 1. Unified HTML Configuration & Stylesheets
# ==========================================
html_theme = 'sphinx_rtd_theme'  # Works exactly the same if using 'furo' or 'alabaster'
html_static_path = ['_static']
html_css_files = [ 'custom.css',]


#
# Line break
#
rst_prolog = """
.. |br| raw:: html

   <br />

.. |br-tex| raw:: latex

   \\vspace{6pt}
"""
