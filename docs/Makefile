defaults = ./config/defaults.yaml
header   = ./config/header.tex

all: Apresentação.pdf Estado-final.pdf avanços

avanços: Avanços-2.pdf \
         Avanços-3.pdf \

%.pdf: %.md $(defaults) $(header)
	pandoc $< -o $@ -s -t beamer \
	    --defaults=$(defaults) \
	    --include-in-header=$(header)

.PHONY: all avanços
