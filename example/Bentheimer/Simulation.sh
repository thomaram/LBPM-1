#!/bin/bash

LBPM_INSTALL_DIR=../../bin

mpirun -np 1 $LBPM_INSTALL_DIR/lbpm_serial_decomp input_Bentheimer.db
#mpirun -np 1 $LBPM_INSTALL_DIR/lbpm_permeability_simulator input_Bentheimer.db
mpirun -np 1 $LBPM_INSTALL_DIR/lbpm_morphopen_pp input_Bentheimer.db
mpirun -np 1 $LBPM_INSTALL_DIR/lbpm_color_simulator input_Bentheimer.db
