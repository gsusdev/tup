TUPlib
=======================

Библиотека поддержки `TUP`.

Структура:
* *tup* - библиотека `TUP`;
	- *inc* - включаемые файлы библиотеки;
		* *tup_instance* - интерфейс пользователя библиотеки;
		* *tup_port* - функции для установки обработчиков запросов библиотеки;
	- *lib* - бинарные файлы библиотеки для подключения к проектам;		
	- *project* - проекты для STM32CubeIDE и QtCreator для сборки;		
	- *src* - исходные тексты;
* *examples* - демонстрационные проекты;
	- *time-but* - проект-решение домашнего задания:
		* *time-but-slave* - проект slave-устройства для STM32F4-DISCOVERY;
		* *time-but-master* - проект master-устройства для STM32F429I-DISCO;
		* *time-but-pc* - проект пользовательского интерфейса для ПК;
* *TupTool* - утилита для отладки библиотеки.

Прямые пути к ключевым текстам:
 - [библиотека - исходники](/tup/src), [библиотека - заголовочные](/tup/inc);
 - [slave-устройство](/examples/time-but/time-but-slave/Core/App);
 - [master-устройство](/examples/time-but/time-but-master/Core/time-but);
 - [интерфейсная утилита для time-but](/examples/time-but/time-but-pc);
 - [отладочная утилита для TUP](/TupTool).

Демонстрационное [видео](https://youtu.be/gyw5UKWxKCU) с платами DISCOVERY.
