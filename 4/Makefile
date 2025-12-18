all: production_line process_resource hospital

production_line:
	gcc -o production_line production_line.c -lpthread -O2

process_resource:
	gcc -o process_resource process_resource.c -lpthread -O2

hospital:
	gcc -o hospital hospital.c -lpthread -O2

clean:
	rm -f production_line process_resource hospital