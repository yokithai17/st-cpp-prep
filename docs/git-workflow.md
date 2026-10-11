# Работа с Git

## Создание основного репозитория

На GitHub создать приватный репозиторий `st-cpp-prep`. Для импорта этих файлов не добавлять README, .gitignore и license через форму: они создадут отдельную начальную историю. Стартовый архив уже содержит README и .gitignore. Лицензию выберем отдельно, если захотим публиковать код.

Структура после распаковки: папка `st-cpp-prep`, внутри неё `README.md`, `CURRENT.md`, `CMakeLists.txt` и остальные файлы. Команды выполняются внутри этой папки.

```bash
git init -b main
git add .
git commit -m "Initialize C++ preparation workspace"
git remote add origin https://github.com/yokithai17/st-cpp-prep.git
git push -u origin main
```

URL выше предназначен для подключённого аккаунта `yokithai17`; если репозиторий создан в другом аккаунте, заменить owner на фактический. Этот адрес не подтверждает существование удалённого репозитория.

Если Git просит имя/email автора, указать собственные данные, используя настройки своего профиля. Для приватного репозитория потребуется GitHub-аутентификация на своём компьютере; пароли, токены и SSH private keys не помещать в файлы проекта и не присылать в чат. Можно использовать HTTPS с настроенным credential manager или SSH.

## Обычное занятие

Перед началом, при чистой рабочей копии:

```bash
git switch main
git pull --ff-only
git switch -c exercise/unique-fd
```

Работа → проверка → небольшой commit:

```bash
git status
git add exercises/01-unique-fd
git commit -m "Implement unique file descriptor ownership"
git push -u origin exercise/unique-fd
```

После этого открыть pull request для ревью. Рекомендованные ветки: `exercise/<name>`, `notes/<topic>`, `fix/<problem>`. Работать над одной основной задачей за раз. Краткий `CURRENT.md`, файл активной задачи `current/CURRENT-<NN>.md` и журнал обновлять вместе с соответствующим результатом. Процесс merge определяем после первого ревью; автоматического слияния в этом стартере нет.

## Минимальные правила

- `main` содержит понятное последнее состояние; экспериментальные изменения — в рабочей ветке.
- Один commit отражает одно осмысленное изменение.
- README объясняет, как запустить задачу; журнал показывает, что действительно проверено.
- Исполняемые файлы и каталоги сборки не коммитить.
- Собственные проекты используют отдельные origin; список находится в `projects/README.md`.

Официальная документация: https://docs.github.com/en/repositories/creating-and-managing-repositories/creating-a-new-repository и https://docs.github.com/en/repositories/creating-and-managing-repositories/cloning-a-repository.
