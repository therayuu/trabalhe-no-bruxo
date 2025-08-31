defaults = ./config/defaults.yaml
header   = ./config/header.tex

guis_therayuu.pdf: Slide.md $(defaults) $(header)
	pandoc $< -o $@ -s -t beamer \
	    --defaults=$(defaults) \
	    --include-in-header=$(header) \
	    #--slide-level=1