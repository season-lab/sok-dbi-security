# SoK: Using Dynamic Binary Instrumentation for Security
This repository hosts the code for the paper "SoK: Using Dynamic Binary Instrumentation for Security (And How You May Get Caught Red Handed)" appeared at ACM Asia CCS 2019 (find the pre-print [here](https://www.diag.uniroma1.it/~delia/papers/asiaccs2019.pdf) or on [ResearchGate](https://www.researchgate.net/publication/332849052_SoK_Using_Dynamic_Binary_Instrumentation_for_Security_And_How_You_May_Get_Caught_Red_Handed)).

The code comprises a library of **mitigations** that can be integrated in existing pintools, and a set of **detections** for DBI that we wrote in addition to those tested with existing PoCs. We share the version used for the evaluation, and we plan to add more countermeasures and our own implementations of other detection patterns.

As this is a research prototype, please get in touch if you encounter issues: we do not expect it to work out of the box in arbitrary scenarios. The library is currently 32-bit only and was tested on Pin 3.5, Windows 7 SP1, and Visual Studio 2010. 

### Developers
* Daniele Cono D'Elia ([@dcdelia](https://github.com/dcdelia))
* Federico Palmaro ([@nik94](https://github.com/nik94))
