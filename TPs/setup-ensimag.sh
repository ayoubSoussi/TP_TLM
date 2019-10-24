#! /bin/echo Non, il ne faut pas executer ce fichier, mais faire: source $0

export SYSTEMCROOT=/matieres/5MMMTSP/tlm/systemc-2.3.2/

# Chaine de cross-compilation MicroBlaze, pour le TP3
xilinx=/matieres/5MMMTSP/tlm/microblaze/setup.sh
if [ -f "$xilinx" ]; then
    source "$xilinx"
fi
