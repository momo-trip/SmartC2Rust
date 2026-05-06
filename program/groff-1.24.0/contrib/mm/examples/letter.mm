.\" To observe how the different `LT` letter types affect document
.\" rendering, define the string `lT` to any of `SB`, `FB`, or `SP`,
.\" when formatting it.  `BL` is the implied default.
.\"
.\" For example...
.\"   $ groff -mm -dlT=SB letter.mm > letter.ps
.do if !d lT .ds lT \" empty
.ND "17 May 2023"
.WA "Epi G. Netic" "Head of Research"
123 Main Street
Anytown, ST  10101
.WE
.IA "Rufus T. Arbogast" "Autovectorization Guru"
456 Elsewhere Avenue
Nirvana, PA  20406
.IE
.LT \*(lT
.P
We have a research leak!
The next person I catch embedding engineering samples of our Lightspeed
Overdrive 2048-core processors in cork coasters distributed at trade
shows is going to regret it.
.FC
.SG
.NS
sundry careless people
.NE
.\" vim: set noexpandtab textwidth=72:
