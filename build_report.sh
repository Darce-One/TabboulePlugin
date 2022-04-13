pandoc README.md -s -o Report.latex
xelatex Report.latex
rm Report.aux Report.log
open Report.pdf
