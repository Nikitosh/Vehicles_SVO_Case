# Кейс Шереметьево, решение команды Retired Z

**Важно:** к моменту стоп-кодинга закончена алгоритмическая часть решения. Инструменты для 
визуализации (с явного разрешения членов жюри на втором чек-поинте) будут закончены ко времени
отправки материалов презентаций.

## Задача

По информации о расписании рейсов и о местах стоянок назначить самолетам места
стоянок таким образом, что суммарная стоимость обработки всех самолетов будет минимальной. 

## Реализованная функциональность
* Решение задачи, которое минимизирует суммарную стоимость в два этапа: 
  * Построение приближенного решения при помощи жадного алгоритма
  * Итеративная оптимизация полученного решения методом имитации отжига

* [WIP] Инструмент для анализа построенного решения в виде интерактивной веб-страницы

 
## Особенности проекта

### Алгоритмическое решение
* Поддержка режима до-оптимизации готового решения
  * Требует только входной файл, совпадающий с тем, что участники отправляли в лидерборд 
  * Может быть реализовано другим алгоритмом
* Поддержка ограничений по времени
* Расширяемость для добавления новых ограничений
  * Достаточно реализации одного метода для добавления одного ограничения
* Расширяемость для тестирования новых гипотез
  * Достаточно реализации одного интерфейса для добавления новых решающих стратегий

### Инструмент для анализа
* Вывод статистики о полученном распределении самолетов по местам стоянки, которая облегчает анализ сильных и слабых 
сторон решения
* Возможность протестировать решение в случае меньшей или болшей загрузки аэропорта
  * Реализована засчет прореживания расписания или же наоборот его обогащения искусственными рейсами 

## Основной стек технологий

* C++, Rapid CSV
* Python, Plotly, Flask, Dash, Pandas

## Демо
* [WIP] Здесь будет ссылка на видео

### Сборка решения

1. Склонируйте репозиторий  
~~~
git clone https://github.com/Nikitosh/Vehicles_SVO_Case.git
~~~
2. Для сборки алгоритмической части решения выполните:
~~~
cd src
g++ -O2 -std=gnu++17 -g solution.cpp -o solution
~~~
3. Для запуска алгоритмической части на приватном датасете выполните, передав первым аргументом ограничение на время 
работы в секундах (результат будет сохранен в `data/private/Solution_Private.csv`):
~~~
./solution time_in_seconds private
~~~
4. Для запуска на новых данных выполните, передав пути до файлов с данными:
~~~
./solution time_in_seconds custom \
	aircraftClassPath \
	aircraftStandsPath \
	handlingRatesPath \
	handlingTimePath \
	timetablePath \
	outputSolutionPath
~~~

### Запуск инструмента для анализа


1. Выполните шаги 1 и 2 из предыдущего раздела для сборки алгоритмического решения 
2. Для запуска инструмента для анализа выполните и перейдите по выданному IP-адресу:
~~~
cd ui
pip install -r requirements.txt
python app.py
~~~

## Команда

* Никита Подгузов [telegram](t.me/Nikitosh)
* Ксения Шор [telegram](t.me/kseniiashor)
* Егор Богомолов [telegram](t.me/ebogomolov)

