# Proyecto de Teoría Computacional

**_Autor_**: _Juan Manuel Alvarado Sandoval_

## Características

- Lee y procesa un archivo .txt que interpreta como un autómata, ya sea un autómata finito determinista o no determinista.

- Ejecuta el algoritmo de 3 pasos asociado a la construcción de Thompson para la conversión de autómatas no deterministas a deterministas.

- Valida y verifica si una cadena pertenece a un autómata finito no determinista que puede tener transiciones épsilon.

- Puede generar la conversión de no determinista a determinista mediante la construcción de subconjuntos.

- Utiliza funciones construidas desde cero, empleando únicamente las bibliotecas stdlib.h y stdio.h.

## Notas de Uso

1.  Se debe seguir la estructura de quíntupla para la interpretación del archivo:
    - Línea 1: Estados (Q)
    - Línea 2: Alfabeto
    - Línea 3: Estado inicial
    - Línea 4: Estados finales
    - Línea 5 y líneas posteriores: Transiciones.
      El archivo de salida se leerá con el mismo orden.
2.  En caso de utilizar transiciones épsilon, coloque el carácter `'E'` al final del alfabeto.
3.  No deje espacios en blanco al redactar el archivo que se leerá.

## Detalles de Implementación

#### Algorítmo: Construcción de Thompson (NFA-DFA)

1. Obtención de conjuntos clave

- Obtiene el conjunto de aquellos estados con transición diferente de epsilon (Target states).
- Obtiene el primer conjunto no mapeado, el cual es equivalente al estado inicial del NFA.
- Excluye a epsilon del alfabeto para el algorítmo.
- Ejecuta una cerradura epsilon especial para el conjunto inicial la cual no es visible para el mapping ni genera transiciones.
  **(primer paso de thompson es aplicar al primer estado)**

2. Mientras la cardinalidad indique que se han encontrado nuevos conjuntos mapeados, ejecuta un while loop para aplicar la funcion go_to a cada estado descubierto **(segundo paso de thompson)**, para cada elemento en el alfabeto para potencialmente generar nuevos conjuntos.
   **(tercer paso de thompson es repetir)**

3. Si el algorítmo no encuentra nuevos conjuntos mapeados entonces imprime el archivo de salida y muestra los mapeos en consola.

##### **Funciones Anidadas y lógica de tipos de dato abstracto**.

Tipos de dato: Se manejo principalmente con la estructura **set** para representar conjuntos y tambien arreglos de conjuntos.

```c
typedef struct {
    string *elements;
    int card;
} set;
```

En orden descendente de abstracción:

**go_to:** Aplica la cerradura epsilon a un conjunto retornado por la función mover relacionada a un elemento del alfabeto.
Está definida como: `go_to` = ε-cerradura(mover(σ<sub>i</sub>))

**Cerradura Epsilon (epsilon_closure):** Se define como la union de los estados originales con el conjunto de estados los cuales recoge como destinos de transiciones epsilon de los estados originales.

1. Ejecuta la operacion de posibilidades epsilon y obtiene un conjunto.
2. Si el conjunto ya existe en la lista de mappings omite el conjunto explorado y lo añade como transición segun un map_index.
3. Si el conjunto no existe lo agrega a la lista de mapeos y le asigna un caracter.
4. Si los indices sigma y map son igual a -1 entonces se crea el primer potencial mapeo sin transiciones.

**Mover a (move_to):** Obtiene los destinos de la intersección del conjunto actual con el conjunto de estados Target (aquellos que tienen una transicion que consume un simbolo del alfabeto y es distinto de epsilon).

#### Algorítmo: Validación de cadena perteneciente a un automata finito no determinista (NFA-ε).

1. Lee una cadena
2. Manda a llamar process_string
3. Imprime la lista de caminos: si encuentra al menos 1 camino válido,entonces la cadena es aceptada por el automata.

Process_string: Consiste en una función recursiva la cual

- Copia los elementos del estado actual al camino actual el cual se está procesando y explorando por la misma recursión, independientemente de las bifurcaciones salva el camino actual.

###### **Caso Base**

- Si la cadena consumió todos sus caracteres verificar si el estado actual es igual al estado final del automata, eso significaría que es una ruta valida y por lo tanto se agrega.

###### **Caso Recursivo**

- Se selecciona un caracter de la cadena y para cada transicion con el estado actual se evaluan los posibles destinos.
  si el simbolo de la transición es epsilon, se manda a llamar la función recursivamente sin consumir ningun caracter de la cadena y mandando el destino de la transicion como estado actual.
  si el simbolo de la transición es diferente de epsilon se manda a llamar a la funcion recursivamente consumiendo un caracter de la cadena.
