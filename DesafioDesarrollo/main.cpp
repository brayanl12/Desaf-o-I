/*
 * Programa demostrativo de manipulaciónprocesamiento de imágenes BMP en C++ usando Qt.
 *
 * Descripción:
 * Este programa realiza las siguientes tareas:
 * 1. Carga una imagen BMP desde un archivo (formato RGB sin usar estructuras ni STL).
 * 2. Modifica los valores RGB de los píxeles asignando un degradado artificial basado en su posición.
 * 3. Exporta la imagen modificada a un nuevo archivo BMP.
 * 4. Carga un archivo de texto que contiene una semilla (offset) y los resultados del enmascaramiento
 *    aplicados a una versión transformada de la imagen, en forma de tripletas RGB.
 * 5. Muestra en consola los valores cargados desde el archivo de enmascaramiento.
 * 6. Gestiona la memoria dinámicamente, liberando los recursos utilizados.
 *
 * Entradas:
 * - Archivo de imagen BMP de entrada ("I_O.bmp").
 * - Archivo de salida para guardar la imagen modificada ("I_D.bmp").
 * - Archivo de texto ("M1.txt") que contiene:
 *     • Una línea con la semilla inicial (offset).
 *     • Varias líneas con tripletas RGB resultantes del enmascaramiento.
 *
 * Salidas:
 * - Imagen BMP modificada ("I_D.bmp").
 * - Datos RGB leídos desde el archivo de enmascaramiento impresos por consola.
 *
 * Requiere:
 * - Librerías Qt para manejo de imágenes (QImage, QString).
 * - No utiliza estructuras ni STL. Solo arreglos dinámicos y memoria básica de C++.
 *
 * Autores: Augusto Salazar Y Aníbal Guerra
 * Fecha: 06/04/2025
 * Asistencia de ChatGPT para mejorar la forma y presentación del código fuente
 */

#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>

using namespace std;
unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);
//CODIGOS(FUCIONES)
void applyXOR(unsigned char* img1, unsigned char* img2, unsigned char* output, int size) {
    for (int i = 0; i < size; ++i) {
        output[i] = img1[i] ^ img2[i];
    }
}
void rotateBitsRight(unsigned char* img, int size, int n_bits) {
    for (int i = 0; i < size; ++i) {
        img[i] = (img[i] >> n_bits) | (img[i] << (8 - n_bits));
    }
}

void rotateBitsLeft(unsigned char* img, int size, int n_bits) {
    for (int i = 0; i < size; ++i) {
        img[i] = (img[i] << n_bits) | (img[i] >> (8 - n_bits));
    }
}

void shiftBitsRight(unsigned char* img, int size, int n_bits) {
    for (int i = 0; i < size; ++i) {
        img[i] = img[i] >> n_bits;
    }
}

void shiftBitsLeft(unsigned char* img, int size, int n_bits) {
    for (int i = 0; i < size; ++i) {
        img[i] = img[i] << n_bits;
    }
}

bool verificarEnmascaramiento(unsigned char* image, unsigned int* maskData, unsigned char* mask, int seed, int maskSize) {
    for (int i = 0; i < maskSize * 3; i++) {
        int pos = seed * 3 + i;
        if (image[pos] + mask[i] != maskData[i]) {
            return false;
        }
    }
    return true;
}
void buscarSecuencia(unsigned char* originalImage, unsigned int* maskingData, int totalPixels, int seed, int n_pixels, int width, int height) {
    int operaciones[] = {0, 1, 2, 3};  // 0: rotate left, 1: rotate right, 2: shift left, 3: shift right
    int bits[] = {1,2,3,4,5,6,7,8};

    for (int op1 = 0; op1 < 4; op1++) {
        for (int bits1 = 0; bits1 < 8; bits1++) {

            unsigned char* temp1 = new unsigned char[totalPixels];
            memcpy(temp1, originalImage, totalPixels);

            // Aplicar primera operación
            switch(op1) {
            case 0: rotateBitsLeft(temp1, totalPixels, bits[bits1]); break;
            case 1: rotateBitsRight(temp1, totalPixels, bits[bits1]); break;
            case 2: shiftBitsLeft(temp1, totalPixels, bits[bits1]); break;
            case 3: shiftBitsRight(temp1, totalPixels, bits[bits1]); break;
            }

            for (int op2 = 0; op2 < 4; op2++) {
                for (int bits2 = 0; bits2 < 8; bits2++) {

                    unsigned char* temp2 = new unsigned char[totalPixels];
                    memcpy(temp2, temp1, totalPixels);

                    // Aplicar segunda operación
                    switch(op2) {
                    case 0: rotateBitsLeft(temp2, totalPixels, bits[bits2]); break;
                    case 1: rotateBitsRight(temp2, totalPixels, bits[bits2]); break;
                    case 2: shiftBitsLeft(temp2, totalPixels, bits[bits2]); break;
                    case 3: shiftBitsRight(temp2, totalPixels, bits[bits2]); break;
                    }

                    // Cargar la máscara de XOR
                    int widthM, heightM;
                    unsigned char* mascara = loadPixels("I_M.bmp", widthM, heightM);
                    if (!mascara) {
                        cout << "Error cargando la máscara." << endl;
                        delete[] temp1;
                        delete[] temp2;
                        return;
                    }

                    // Aplicar XOR
                    applyXOR(temp2, mascara, temp2, totalPixels);

                    // Verificar
                    if (verificarEnmascaramiento(temp2, maskingData, mascara, seed, n_pixels)) {
                        cout << "¡Encontrada secuencia correcta!" << endl;
                        cout << "Primera operación: " << (op1==0?"Rotate Left":op1==1?"Rotate Right":op1==2?"Shift Left":"Shift Right") << " con " << bits[bits1] << " bits" << endl;
                        cout << "Segunda operación: " << (op2==0?"Rotate Left":op2==1?"Rotate Right":op2==2?"Shift Left":"Shift Right") << " con " << bits[bits2] << " bits" << endl;
                        exportImage(temp2, width, height, "Imagen_Recuperada.bmp");
                        delete[] temp1;
                        delete[] temp2;
                        delete[] mascara;
                        return;
                    }

                    delete[] temp2;
                    delete[] mascara;
                }
            }
            delete[] temp1;
        }
    }
    cout << "No se encontró ninguna combinación correcta." << endl;
}

//FIN

int main()
{
    // Definición de rutas de archivo de entrada (imagen original) y salida (imagen modificada)
    QString archivoEntrada = "I_O.bmp";
    QString archivoSalida = "I_D.bmp";

    // Variables para almacenar las dimensiones de la imagen
    int height = 0;
    int width = 0;

    // Carga la imagen BMP en memoria dinámica y obtiene ancho y alto
    unsigned char *pixelData = loadPixels(archivoEntrada, width, height);
    if (!pixelData) return -1;

    int totalPixels = width * height * 3;

    bool exportI = exportImage(pixelData, width, height, archivoSalida);

    cout << exportI << endl;

    // Variables para almacenar la semilla y el número de píxeles leídos del archivo de enmascaramiento
    int seed = 0;
    int n_pixels = 0;

    // Carga los datos de enmascaramiento desde un archivo .txt (semilla + valores RGB)
    unsigned int *maskingData = loadSeedMasking("M.bmp", seed, n_pixels);
    if (!maskingData) { delete[] pixelData; return -1; }

    // Ejemplo de máscara M simulada (en la práctica debes cargarla igual que las imágenes)
    unsigned char* mascara = new unsigned char[n_pixels * 3];
    for (int i = 0; i < n_pixels * 3; i++) mascara[i] = 0;


    for (int i = 0; i < n_pixels * 3; i += 3) {
        cout << "Pixel " << i / 3 << ": ("
             << maskingData[i] << ", "
             << maskingData[i + 1] << ", "
             << maskingData[i + 2] << ")" << endl;
    }


    // Suponiendo que tenemos varios pasos a invertir
    int posiblesBits[] = {1, 2, 3, 4, 5, 6, 7, 8}; // bits posibles en rotaciones

    for (int b = 0; b < 8; b++) {  // probar con cada cantidad de bits
        unsigned char* imagenTemp = new unsigned char[totalPixels];
        memcpy(imagenTemp, pixelData, totalPixels);  // Copia de la distorsionada ID

        // Invertir la última transformación conocida
        rotateBitsLeft(imagenTemp, totalPixels, posiblesBits[b]);  // Suponiendo que la última fue rotación derecha

        // Invertir la anterior, por ejemplo un XOR con IM
        unsigned char* imagenIM = loadPixels("I_M.bmp", width, height);

        if (!imagenIM) {
            cout << "Error: No se pudo cargar I_M.bmp." << endl;
            delete[] imagenTemp;
            continue;
        }

        applyXOR(imagenTemp, imagenIM, imagenTemp, totalPixels);  // XOR otra vez para revertir

        delete[] imagenIM;

        // Verificar enmascaramiento
        bool coincide = verificarEnmascaramiento(imagenTemp, maskingData, mascara, seed, n_pixels);
        if (coincide) {
            cout << "Secuencia correcta encontrada con rotacion " << posiblesBits[b] << " bits!" << endl;
            delete[] imagenTemp;
            break;
        }
        delete[] imagenTemp;
    }

    bool esCorrecto = verificarEnmascaramiento(pixelData, maskingData, mascara, seed, n_pixels);
    if(esCorrecto) {
        cout << "Enmascaramiento verificado: La transformacion coincide!" << endl;
    } else {
        cout << "No coincide con el archivo de enmascaramiento." << endl;
    }
    buscarSecuencia(pixelData, maskingData, width * height * 3, seed, n_pixels, width, height);

    // === Liberar memoria ===
    delete[] pixelData;
    delete[] maskingData;
    delete[] mascara;


    return 0; // Fin del programa
}
//FUNCIONES (CODIGOS)



//FIN



  // Se asume que maskingData contiene los valores RGB de la máscara y tiene el mismo tamaño que pixelData



unsigned char* loadPixels(QString input, int &width, int &height){
/*
 * @brief Carga una imagen BMP desde un archivo y extrae los datos de píxeles en formato RGB.
 *
 * Esta función utiliza la clase QImage de Qt para abrir una imagen en formato BMP, convertirla al
 * formato RGB888 (24 bits: 8 bits por canal), y copiar sus datos de píxeles a un arreglo dinámico
 * de tipo unsigned char. El arreglo contendrá los valores de los canales Rojo, Verde y Azul (R, G, B)
 * de cada píxel de la imagen, sin rellenos (padding).
 *
 * @param input Ruta del archivo de imagen BMP a cargar (tipo QString).
 * @param width Parámetro de salida que contendrá el ancho de la imagen cargada (en píxeles).
 * @param height Parámetro de salida que contendrá la altura de la imagen cargada (en píxeles).
 * @return Puntero a un arreglo dinámico que contiene los datos de los píxeles en formato RGB.
 *         Devuelve nullptr si la imagen no pudo cargarse.
 *
 * @note Es responsabilidad del usuario liberar la memoria asignada al arreglo devuelto usando `delete[]`.
 */

    // Cargar la imagen BMP desde el archivo especificado (usando Qt)
    QImage imagen(input);

    // Verifica si la imagen fue cargada correctamente
    if (imagen.isNull()) {
        cout << "Error: No se pudo cargar la imagen BMP." << std::endl;
        return nullptr; // Retorna un puntero nulo si la carga falló
    }

    // Convierte la imagen al formato RGB888 (3 canales de 8 bits sin transparencia)
    imagen = imagen.convertToFormat(QImage::Format_RGB888);

    // Obtiene el ancho y el alto de la imagen cargada
    width = imagen.width();
    height = imagen.height();

    // Calcula el tamaño total de datos (3 bytes por píxel: R, G, B)
    int dataSize = width * height * 3;

    // Reserva memoria dinámica para almacenar los valores RGB de cada píxel
    unsigned char* pixelData = new unsigned char[dataSize];

    // Copia cada línea de píxeles de la imagen Qt a nuestro arreglo lineal
    for (int y = 0; y < height; ++y) {
        const uchar* srcLine = imagen.scanLine(y);              // Línea original de la imagen con posible padding
        unsigned char* dstLine = pixelData + y * width * 3;     // Línea destino en el arreglo lineal sin padding
        memcpy(dstLine, srcLine, width * 3);                    // Copia los píxeles RGB de esa línea (sin padding)
    }

    // Retorna el puntero al arreglo de datos de píxeles cargado en memoria
    return pixelData;
}

bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida){
/*
 * @brief Exporta una imagen en formato BMP a partir de un arreglo de píxeles en formato RGB.
 *
 * Esta función crea una imagen de tipo QImage utilizando los datos contenidos en el arreglo dinámico
 * `pixelData`, que debe representar una imagen en formato RGB888 (3 bytes por píxel, sin padding).
 * A continuación, copia los datos línea por línea a la imagen de salida y guarda el archivo resultante
 * en formato BMP en la ruta especificada.
 *
 * @param pixelData Puntero a un arreglo de bytes que contiene los datos RGB de la imagen a exportar.
 *                  El tamaño debe ser igual a width * height * 3 bytes.
 * @param width Ancho de la imagen en píxeles.
 * @param height Alto de la imagen en píxeles.
 * @param archivoSalida Ruta y nombre del archivo de salida en el que se guardará la imagen BMP (QString).
 *
 * @return true si la imagen se guardó exitosamente; false si ocurrió un error durante el proceso.
 *
 * @note La función no libera la memoria del arreglo pixelData; esta responsabilidad recae en el usuario.
 */

    // Crear una nueva imagen de salida con el mismo tamaño que la original
    // usando el formato RGB888 (3 bytes por píxel, sin canal alfa)
    QImage outputImage(width, height, QImage::Format_RGB888);

    // Copiar los datos de píxeles desde el buffer al objeto QImage
    for (int y = 0; y < height; ++y) {
        // outputImage.scanLine(y) devuelve un puntero a la línea y-ésima de píxeles en la imagen
        // pixelData + y * width * 3 apunta al inicio de la línea y-ésima en el buffer (sin padding)
        // width * 3 son los bytes a copiar (3 por píxel)
        memcpy(outputImage.scanLine(y), pixelData + y * width * 3, width * 3);
    }

    // Guardar la imagen en disco como archivo BMP
    if (!outputImage.save(archivoSalida, "BMP")) {
        // Si hubo un error al guardar, mostrar mensaje de error
        cout << "Error: No se pudo guardar la imagen BMP modificada.";
        return false; // Indica que la operación falló
    } else {
        // Si la imagen fue guardada correctamente, mostrar mensaje de éxito
        cout << "Imagen BMP modificada guardada como " << archivoSalida.toStdString() << endl;
        return true; // Indica éxito
    }

}

unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels){
/*
 * @brief Carga la semilla y los resultados del enmascaramiento desde un archivo de texto.
 *
 * Esta función abre un archivo de texto que contiene una semilla en la primera línea y,
 * a continuación, una lista de valores RGB resultantes del proceso de enmascaramiento.
 * Primero cuenta cuántos tripletes de píxeles hay, luego reserva memoria dinámica
 * y finalmente carga los valores en un arreglo de enteros.
 *
 * @param nombreArchivo Ruta del archivo de texto que contiene la semilla y los valores RGB.
 * @param seed Variable de referencia donde se almacenará el valor entero de la semilla.
 * @param n_pixels Variable de referencia donde se almacenará la cantidad de píxeles leídos
 *                 (equivalente al número de líneas después de la semilla).
 *
 * @return Puntero a un arreglo dinámico de enteros que contiene los valores RGB
 *         en orden secuencial (R, G, B, R, G, B, ...). Devuelve nullptr si ocurre un error al abrir el archivo.
 *
 * @note Es responsabilidad del usuario liberar la memoria reservada con delete[].
 */

    // Abrir el archivo que contiene la semilla y los valores RGB
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        // Verificar si el archivo pudo abrirse correctamente
        cout << "No se pudo abrir el archivo." << endl;
        return nullptr;
    }

    // Leer la semilla desde la primera línea del archivo
    archivo >> seed;

    int r, g, b;

    // Contar cuántos grupos de valores RGB hay en el archivo
    // Se asume que cada línea después de la semilla tiene tres valores (r, g, b)
    while (archivo >> r >> g >> b) {
        n_pixels++;  // Contamos la cantidad de píxeles
    }

    // Cerrar el archivo para volver a abrirlo desde el inicio
    archivo.close();
    archivo.open(nombreArchivo);

    // Verificar que se pudo reabrir el archivo correctamente
    if (!archivo.is_open()) {
        cout << "Error al reabrir el archivo." << endl;
        return nullptr;
    }

    // Reservar memoria dinámica para guardar todos los valores RGB
    // Cada píxel tiene 3 componentes: R, G y B
    unsigned int* RGB = new unsigned int[n_pixels * 3];

    // Leer nuevamente la semilla desde el archivo (se descarta su valor porque ya se cargó antes)
    archivo >> seed;

    // Leer y almacenar los valores RGB uno por uno en el arreglo dinámico
    for (int i = 0; i < n_pixels * 3; i += 3) {
        archivo >> r >> g >> b;
        RGB[i] = r;
        RGB[i + 1] = g;
        RGB[i + 2] = b;
    }

    // Cerrar el archivo después de terminar la lectura
    archivo.close();

    // Mostrar información de control en consola
    cout << "Semilla: " << seed << endl;
    cout << "Cantidad de píxeles leídos: " << n_pixels << endl;

    // Retornar el puntero al arreglo con los datos RGB
    return RGB;
}













