defaults = ./config/defaults.yaml
header   = ./config/header.tex

avanços: Avanços-2.pdf Avanços-3.pdf Avanços-4.pdf

all: Apresentação.pdf avanços

%.pdf: %.md $(defaults) $(header)
	pandoc $< -o $@ -s -t beamer \
	    --defaults=$(defaults) \
	    --include-in-header=$(header)


Apresentação.pdf: Apresentação.md $(defaults) $(header)
	pandoc $< -o $@ -s -t beamer \
	    --defaults=$(defaults) \
	    --include-in-header=$(header) \
	    #--slide-level=1
