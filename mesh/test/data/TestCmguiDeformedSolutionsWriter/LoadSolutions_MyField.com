#
# Cmgui script automatically generated by Chaste
#
for ($i=0; $i<=2; $i++) { 
  gfx read node solution_$i time $i
 gfx read node myfield_$i time $i
}
gfx read ele solution_0
gfx define faces egroup solution
gfx cr win

