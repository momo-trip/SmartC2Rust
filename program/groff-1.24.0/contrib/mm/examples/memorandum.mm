.\" To observe how the different `MT` memorandum types affect document
.\" rendering, define the string `mT` to any integer from 0-5 inclusive
.\" or arbitrarily when formatting it.  `1` is the implied default.
.\"
.\" For example...
.\"   $ groff -mm -dmT=2 memorandum.mm > memorandum.ps
.\"   $ groff -mm -dmT=ATTENTION memorandum.mm > memorandum.ps
.do if !d mT .ds mT \" empty
.if n .SA 1
.AF "Yoyodyne, Inc."
.TL 123 456
A Blowdown Stack Modification to the Turbo Encabulator
.AU "Art Vandelay" axv C D E F G H I
.AT "President" "501(c)6 Convenor"
.AU "H.\& E.\& Pennypacker" hep J K L M N O P
.AT "Chairman" "Wealthy Industrialist"
.TM 78-9-ABC 98-7-DEF
.AS
We're changing the world,
one obsolescently planned gizmo at a time.
.AE
.ND 2024-06-12
.MT \*(mT
Successful leverage of our core competencies to achieve economies of
scale has transformed our entire sector of industry with exciting new
synergies in allocating more money to (already rich) people.
.SG QRS
.NS
A.\& Pratt
B.\& Sharpe
.NE
