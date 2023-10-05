# Bitmap Lauflängenkodierung

Komprimiere Bitmaps über die Lauflängenkodierung (run-length-encoding)

## Implementierung

### Installation

Nutze GNU Make um eine Executable `bmpRle` zu erstellen.
```bash
cd ./Implementierung
make
```

### Ausführung

Die Optionen und Argumente werden entsprechend gesetzt:
```bash
./bmpRle -V <int> -B <int> -o <output.bmp> <input.bmp>
```

Die Standardausführung entspricht
```bash
./bmpRle input.bmp
```
dabei werden Default Argumente für die Optionen gesetzt, die Ausführung entspricht:
```bash
./bmpRle -V0 -o out.bmp input.bmp
```

### Optionen

| Option     | Argument                                                      | Default   | Beschreibung       |
|------------|---------------------------------------------------------------|-----------|----------------------------------------------------------------------------------------------------------------|
| -V         | ja, eine Version in [0,3]                                     | 0         | Spezifiziert die verwendete Version |
| -B         | ja, Anzahl der zu messenden Wiederholungen                    | 0         | Misst die Laufzeit der RLE-Komprimierung, wenn spezifiziert
| -o         | ja, Pfad zur Ausgabedatei                                     | ./out.bmp | Spezifiziert die Ausgabedatei
| -h, --help | nein                                                          | -         | Gibt Beschreibung aller Optionen des Programms und Verwendungsbeispiele aus. Das Programm beendet sich danach. | 

### Weitere Beispielausführung

Nutze Version 1 für die Komprimierung
```bash
./bmpRle -V1 ./bitmap_examples/deer_7C_397x706.bmp
```

Nutze Version 0 und miss die Zeit der Komprimierung 6-mal
```bash
./bmpRle -B5 ./bitmap_examples/lena_7C_512x512.bmp
```

Nutze Version 2 und miss die Zeit der Komprimierung 5-mal, schreibe die komprimierte Bitmap in 'new.bmp'
```bash
./bmpRle -V2 -B4 -o ./new.bmp ./bitmap_examples/lena_7C_512x512.bmp
```

Zeige die Hilfe an
```bash
./bmpRle --help
```

### Versionen

V0 bis V2 verwenden den Absolute und Encoded Mode des Bitmap Formats für eine bessere Komprimierung.<br>
V3 verwendet nur den Encoded Mode des Bitmap Formats.

| Version | Beschreibung                                                                        |
|---------|-------------------------------------------------------------------------------------|
| V0      | eine mit SIMD optimierte Version                                                    |
| V1      | Alternativimplementierung 1                                                         |
| V2      | Alternativimplementierung 2                                                         |     
| V3      | Alternativimplementierung 3, verwendet nur Encode Mode                              |

### Beispiele
Im Ordner `./bitmap_examples` befinden sich Bitmap Dateien in verschiedenen Information Header Größen die komprimiert werden können.

## Mitwirkende

- Adam Karamelo
- Philipp Czernitzki
