metaballs:
	cc metaballs.c -Wall -o metaballs -lm

clean:
	rm -rf ./jgrs
	rm -rf ./frames
	rm ./metaballs

run:
	./metaballs 50
	mkdir -p ./jgrs
	mkdir -p ./frames
	( \
		i=0; \
		for F in ./jgrs/*; \
			do \
			./jgraph -P $$F \
			| ps2pdf - \
			| convert -density 300 - -quality 100 $$(printf "./frames/frame%05d.jpg" $$i); \
			i=$$((i+1)); \
		done; \
	)
	ffmpeg -y -framerate 10 -i frames/frame%05d.jpg -c:v libx264 -profile:v high -crf 20 -pix_fmt yuv420p output.mp4
