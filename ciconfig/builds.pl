# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

$builds = {
	   # 60 => {
	   # 	  buildtool => "g61_latest",
	   # 	  hextool => "6.1",
	   # 	  targets => ["ref -fno-inline", "opt -fno-inline", "ref", "opt",
	   # 		      "opt H2K_LOAD_ADDR=0x00400000 H2K_EXTRA_CFLAGS=-DNMI_STOP"],
	   # 	  cov => ["ref -fno-inline", "opt -fno-inline"],
	   # 	  docs => "opt",
	   # 	  release => ["opt","opt H2K_LOAD_ADDR=0x00400000 H2K_EXTRA_CFLAGS=-DNMI_STOP"],
	   # 	  branch => ["master", "stable"],
	   # 	 },
	   65 => {
		  buildtool => "g61_latest",
		  hextool => "6.1",
		  targets => ["ref -fno-inline", "opt -fno-inline", "ref", "opt",
			      "opt H2K_LOAD_ADDR=0x00400000 H2K_EXTRA_CFLAGS=-DNMI_STOP",
			      "ref H2K_LOAD_ADDR=0x00400000 H2K_EXTRA_CFLAGS=-DNMI_STOP"],
		  cov => ["ref -fno-inline", "opt -fno-inline"],
		  docs => "opt",
		  release => ["opt","opt H2K_LOAD_ADDR=0x00400000 H2K_EXTRA_CFLAGS=-DNMI_STOP",
			      "ref","ref H2K_LOAD_ADDR=0x00400000 H2K_EXTRA_CFLAGS=-DNMI_STOP"],
		  branch => ["master", "stable", "multicore"],
		 },
	   "66t" => {
                     buildtool => "g61_latest",
                     hextool => "6.1",
                     targets => ["opt ARCHV=65 TINY_CORE=1 H2K_LOAD_ADDR=0x00400000 H2K_EXTRA_CFLAGS=-DNMI_STOP"],
                     cov => [],
                     docs => "opt",
                     release => ["opt ARCHV=65 TINY_CORE=1 H2K_LOAD_ADDR=0x00400000 H2K_EXTRA_CFLAGS=-DNMI_STOP"],
                     branch => ["master", "stable"],
                    },
	  };
