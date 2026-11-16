#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PRODUCTOS 5
#define MAX_NOMBRE 100
#define MAX_LINEA 256

/* Arreglos paralelos: el indice identifica los datos de cada producto. */
static char nombres[MAX_PRODUCTOS][MAX_NOMBRE];
static int demandas[MAX_PRODUCTOS];
static double tiempoUnitario[MAX_PRODUCTOS];
static double recursosUnitarios[MAX_PRODUCTOS];
static int cantidadProductos = 0;

static void quitarEspacios(char *texto) {
    size_t inicio = 0;
    size_t longitud = strlen(texto);

    while (inicio < longitud && isspace((unsigned char)texto[inicio])) {
        inicio++;
    }
    while (longitud > inicio && isspace((unsigned char)texto[longitud - 1])) {
        longitud--;
    }
    if (inicio > 0) {
        memmove(texto, texto + inicio, longitud - inicio);
    }
    texto[longitud - inicio] = '\0';
}

/* Lee una linea completa, admite espacios y descarta entradas demasiado largas. */
static int leerLinea(const char *mensaje, char *destino, size_t capacidad) {
    char *salto;

    for (;;) {
        size_t longitud;
        int caracter;

        printf("%s", mensaje);
        fflush(stdout);
        if (fgets(destino, (int)capacidad, stdin) == NULL) {
            return 0;
        }

        salto = strchr(destino, '\n');
        if (salto != NULL) {
            *salto = '\0';
        } else if (!feof(stdin)) {
            while ((caracter = getchar()) != '\n' && caracter != EOF) {
                /* Vaciar el resto de la linea. */
            }
            printf("Entrada demasiado larga. Intente nuevamente.\n");
            continue;
        }

        longitud = strlen(destino);
        if (longitud > 0 && destino[longitud - 1] == '\r') {
            destino[longitud - 1] = '\0';
        }
        quitarEspacios(destino);
        return 1;
    }
}

static int leerTextoNoVacio(const char *mensaje, char *destino, size_t capacidad) {
    char linea[MAX_LINEA];

    for (;;) {
        if (!leerLinea(mensaje, linea, sizeof(linea))) {
            return 0;
        }
        if (linea[0] == '\0') {
            printf("El valor no puede estar vacio.\n");
            continue;
        }
        if (strlen(linea) >= capacidad) {
            printf("El texto es demasiado largo (maximo %u caracteres UTF-8 en bytes).\n",
                   (unsigned)(capacidad - 1));
            continue;
        }
        strcpy(destino, linea);
        return 1;
    }
}

static int leerEnteroNoNegativo(const char *mensaje, int *valor) {
    char linea[MAX_LINEA];

    for (;;) {
        char *fin;
        long numero;

        if (!leerLinea(mensaje, linea, sizeof(linea))) {
            return 0;
        }
        errno = 0;
        numero = strtol(linea, &fin, 10);
        if (linea[0] == '\0' || fin == linea || *fin != '\0' || errno == ERANGE ||
            numero < 0 || numero > INT_MAX) {
            printf("Ingrese un numero entero valido mayor o igual que cero.\n");
            continue;
        }
        *valor = (int)numero;
        return 1;
    }
}

static int leerRealNoNegativo(const char *mensaje, double *valor) {
    char linea[MAX_LINEA];

    for (;;) {
        char *fin;
        double numero;

        if (!leerLinea(mensaje, linea, sizeof(linea))) {
            return 0;
        }
        errno = 0;
        numero = strtod(linea, &fin);
        if (linea[0] == '\0' || fin == linea || *fin != '\0' || errno == ERANGE ||
            !(numero >= 0.0) || numero > DBL_MAX) {
            printf("Ingrese un numero valido mayor o igual que cero (use punto decimal).\n");
            continue;
        }
        *valor = numero;
        return 1;
    }
}

/* Comparacion sin distinguir mayusculas ASCII; los demas bytes UTF-8 se comparan exactos. */
static int nombresIguales(const char *a, const char *b) {
    while (*a != '\0' && *b != '\0') {
        unsigned char ca = (unsigned char)*a;
        unsigned char cb = (unsigned char)*b;
        if (ca < 128 && cb < 128) {
            ca = (unsigned char)tolower(ca);
            cb = (unsigned char)tolower(cb);
        }
        if (ca != cb) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

static int buscarProducto(const char *nombre) {
    int i;
    for (i = 0; i < cantidadProductos; i++) {
        if (nombresIguales(nombres[i], nombre)) {
            return i;
        }
    }
    return -1;
}

static int nombreYaExiste(const char *nombre, int indiceExcluido) {
    int i;
    for (i = 0; i < cantidadProductos; i++) {
        if (i != indiceExcluido && nombresIguales(nombres[i], nombre)) {
            return 1;
        }
    }
    return 0;
}

static void mostrarMenu(void) {
    printf("\n=== CONTROL DE PRODUCCION DE LA FABRICA ===\n");
    printf("1. Registrar producto\n");
    printf("2. Mostrar productos y requerimientos\n");
    printf("3. Editar producto\n");
    printf("4. Eliminar producto\n");
    printf("5. Evaluar viabilidad de produccion\n");
    printf("6. Salir\n");
}

static int registrarProducto(void) {
    char nombre[MAX_NOMBRE];
    int demanda;
    double tiempo;
    double recursos;

    if (cantidadProductos == MAX_PRODUCTOS) {
        printf("Se alcanzo el limite de %d productos.\n", MAX_PRODUCTOS);
        return 1;
    }
    if (!leerTextoNoVacio("Nombre del producto: ", nombre, sizeof(nombre)) ||
        !leerEnteroNoNegativo("Cantidad demandada (unidades): ", &demanda) ||
        !leerRealNoNegativo("Tiempo de fabricacion por unidad (horas): ", &tiempo) ||
        !leerRealNoNegativo("Recursos necesarios por unidad: ", &recursos)) {
        return 0;
    }
    if (nombreYaExiste(nombre, -1)) {
        printf("Ya existe un producto con ese nombre. No se registro.\n");
        return 1;
    }

    strcpy(nombres[cantidadProductos], nombre);
    demandas[cantidadProductos] = demanda;
    tiempoUnitario[cantidadProductos] = tiempo;
    recursosUnitarios[cantidadProductos] = recursos;
    cantidadProductos++;
    printf("Producto registrado correctamente. (%d de %d)\n", cantidadProductos, MAX_PRODUCTOS);
    return 1;
}

static void mostrarProductos(void) {
    int i;
    double tiempoTotal = 0.0;
    double recursosTotales = 0.0;

    if (cantidadProductos == 0) {
        printf("No hay productos registrados.\n");
        return;
    }

    printf("\n=== PRODUCTOS REGISTRADOS ===\n");
    for (i = 0; i < cantidadProductos; i++) {
        double t = (double)demandas[i] * tiempoUnitario[i];
        double r = (double)demandas[i] * recursosUnitarios[i];
        printf("%d. %s | Demanda: %d | Tiempo/unidad: %.3f h | Recursos/unidad: %.3f\n",
               i + 1, nombres[i], demandas[i], tiempoUnitario[i], recursosUnitarios[i]);
        printf("   Requerido: %.3f horas y %.3f unidades de recursos\n", t, r);
        tiempoTotal += t;
        recursosTotales += r;
    }
    printf("Totales requeridos: %.3f horas y %.3f unidades de recursos.\n",
           tiempoTotal, recursosTotales);
}

static int editarProducto(void) {
    char busqueda[MAX_NOMBRE];
    char nuevoNombre[MAX_NOMBRE];
    int indice;
    int nuevaDemanda;
    double nuevoTiempo;
    double nuevosRecursos;

    if (cantidadProductos == 0) {
        printf("No hay productos para editar.\n");
        return 1;
    }
    if (!leerTextoNoVacio("Nombre actual del producto a buscar: ", busqueda, sizeof(busqueda))) {
        return 0;
    }
    indice = buscarProducto(busqueda);
    if (indice < 0) {
        printf("No se encontro el producto '%s'.\n", busqueda);
        return 1;
    }

    printf("Editando '%s'. Ingrese los nuevos datos.\n", nombres[indice]);
    if (!leerTextoNoVacio("Nuevo nombre: ", nuevoNombre, sizeof(nuevoNombre)) ||
        !leerEnteroNoNegativo("Nueva cantidad demandada (unidades): ", &nuevaDemanda) ||
        !leerRealNoNegativo("Nuevo tiempo de fabricacion por unidad (horas): ", &nuevoTiempo) ||
        !leerRealNoNegativo("Nuevos recursos necesarios por unidad: ", &nuevosRecursos)) {
        return 0;
    }
    if (nombreYaExiste(nuevoNombre, indice)) {
        printf("Ya existe otro producto con ese nombre. No se aplicaron los cambios.\n");
        return 1;
    }

    strcpy(nombres[indice], nuevoNombre);
    demandas[indice] = nuevaDemanda;
    tiempoUnitario[indice] = nuevoTiempo;
    recursosUnitarios[indice] = nuevosRecursos;
    printf("Producto actualizado correctamente.\n");
    return 1;
}

static int eliminarProducto(void) {
    char busqueda[MAX_NOMBRE];
    int indice;
    int i;

    if (cantidadProductos == 0) {
        printf("No hay productos para eliminar.\n");
        return 1;
    }
    if (!leerTextoNoVacio("Nombre del producto a eliminar: ", busqueda, sizeof(busqueda))) {
        return 0;
    }
    indice = buscarProducto(busqueda);
    if (indice < 0) {
        printf("No se encontro el producto '%s'.\n", busqueda);
        return 1;
    }

    for (i = indice; i < cantidadProductos - 1; i++) {
        strcpy(nombres[i], nombres[i + 1]);
        demandas[i] = demandas[i + 1];
        tiempoUnitario[i] = tiempoUnitario[i + 1];
        recursosUnitarios[i] = recursosUnitarios[i + 1];
    }
    cantidadProductos--;
    nombres[cantidadProductos][0] = '\0';
    demandas[cantidadProductos] = 0;
    tiempoUnitario[cantidadProductos] = 0.0;
    recursosUnitarios[cantidadProductos] = 0.0;
    printf("Producto eliminado. Ahora puede registrar otro en el espacio disponible.\n");
    return 1;
}

static int evaluarViabilidad(void) {
    int i;
    double tiempoDisponible;
    double recursosDisponibles;
    double tiempoTotal = 0.0;
    double recursosTotales = 0.0;

    if (cantidadProductos == 0) {
        printf("Registre al menos un producto antes de evaluar la produccion.\n");
        return 1;
    }
    if (!leerRealNoNegativo("Tiempo total disponible en la fabrica (horas): ", &tiempoDisponible) ||
        !leerRealNoNegativo("Cantidad total de recursos disponibles: ", &recursosDisponibles)) {
        return 0;
    }

    for (i = 0; i < cantidadProductos; i++) {
        double t = (double)demandas[i] * tiempoUnitario[i];
        double r = (double)demandas[i] * recursosUnitarios[i];
        if (!(t >= 0.0) || t > DBL_MAX || !(r >= 0.0) || r > DBL_MAX || !(tiempoTotal + t >= 0.0) || tiempoTotal + t > DBL_MAX || !(recursosTotales + r >= 0.0) || recursosTotales + r > DBL_MAX) {
            printf("No se puede calcular: los valores producen un resultado demasiado grande.\n");
            return 1;
        }
        tiempoTotal += t;
        recursosTotales += r;
    }

    printf("\n=== RESULTADO DE VIABILIDAD ===\n");
    printf("Tiempo requerido: %.3f h | Disponible: %.3f h\n", tiempoTotal, tiempoDisponible);
    printf("Recursos requeridos: %.3f | Disponibles: %.3f\n", recursosTotales, recursosDisponibles);

    if (tiempoTotal <= tiempoDisponible && recursosTotales <= recursosDisponibles) {
        printf("VIABLE: la fabrica puede cumplir la demanda con el tiempo y los recursos disponibles.\n");
    } else {
        printf("NO VIABLE: no alcanza el tiempo, los recursos o ambos.\n");
        if (tiempoTotal > tiempoDisponible) {
            printf("Faltan %.3f horas.\n", tiempoTotal - tiempoDisponible);
        }
        if (recursosTotales > recursosDisponibles) {
            printf("Faltan %.3f unidades de recursos.\n", recursosTotales - recursosDisponibles);
        }
    }
    return 1;
}

int main(void) {
    char linea[MAX_LINEA];
    int opcion;

    for (;;) {
        char *fin;
        long seleccion;

        mostrarMenu();
        if (!leerLinea("Seleccione una opcion: ", linea, sizeof(linea))) {
            printf("\nFin de la entrada. Programa terminado.\n");
            break;
        }
        errno = 0;
        seleccion = strtol(linea, &fin, 10);
        if (linea[0] == '\0' || fin == linea || *fin != '\0' || errno == ERANGE ||
            seleccion < 1 || seleccion > 6) {
            printf("Opcion invalida. Seleccione un numero del 1 al 6.\n");
            continue;
        }
        opcion = (int)seleccion;

        switch (opcion) {
            case 1:
                if (!registrarProducto()) return 0;
                break;
            case 2:
                mostrarProductos();
                break;
            case 3:
                if (!editarProducto()) return 0;
                break;
            case 4:
                if (!eliminarProducto()) return 0;
                break;
            case 5:
                if (!evaluarViabilidad()) return 0;
                break;
            case 6:
                printf("Programa finalizado.\n");
                return 0;
        }
    }
    return 0;
}






