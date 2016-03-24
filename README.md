﻿# 3d-reconstruction

## Требования

- Qt >= 5.4
- КОмпилятор с поддержкой С++ 11 (сборка проверялась на MinGW, Visual C++, GCC)

Для запуска исполняемого файла из `bin/`:
- Visual C++ Redistributable Packages for Visual Studio 2013 (https://www.microsoft.com/en-au/download/details.aspx?id=40784)

## Описание интерфейса
![screenshot.png](https://github.com/almikh/3d-reconstruction/blob/master/screenshot.png "Скриншот программы")

0. Рабочая область.
1. Включает режим реконструкции модели из изображения. Сами действия для непосредственно реконструкции совершаются ЛКМ.
2. Включает режим выделения модели. Выделение также осуществляется ЛКМ, можно использовать либо выделение с помощью указания прямоугольника, попавшие в который объекты становятся выделенными, либо просто кликнув по желаемому объекту. Именно выделенные объекты можно перемещать, трансформировать и т.п.
3. Триангулировать "верхнюю" крышку модели.
4. Триангулировать "нижнюю" крышку модели.
5. Отменить последнее совершенное пользователем действие.
6. Создать копию выделенного объекта, в той же позиции, что и выделенные меш(и).
7. Объединяет выделенные меши в один путем слияния их своими "крышками".
8. Включает/отключает режим текстурирования реконструируемой модели (доступен список выбора режима текстурирования).
9. Шаг между последовательными слоями (расстояние между ними).
10. Число разбиений круга-основания модели (т.е. весь круг в 360 градусов делится на указанное количество "ломтиков").

> Осуществление перемещения модели происходит с помощью ПКМ. Предварительно целевые объекты должны быть выделены.
