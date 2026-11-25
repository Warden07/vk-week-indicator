# Выпуск обновлений

Готовая первая прошивка и полный комплект: [выпуск 1.0.0](https://github.com/Warden07/vk-week-indicator/releases/tag/v1.0.0). Сборка прошла; физические испытания ещё требуются.

1. Увеличьте FW_BUILD и FW_VERSION в AppConfig.h, соберите и проверьте образ.
2. Подпишите его tools/sign_release.py с локальным закрытым ключом. Ключ никогда не загружайте в GitHub.
3. Загрузите подписанный образ в firmware/ota/<FW_BUILD>/firmware.bin.signed. Проверьте доступ без входа.
4. Только затем увеличьте firmware/ota/build.txt. Не заменяйте образ внутри существующего номера.

Пример: `python tools/sign_release.py --key /private/path/private.key --input firmware.bin --output firmware/ota/2/firmware.bin.signed`. Требуется пакет cryptography.

Первую установку и восстановление выполняйте по USB. Автоматического отката нет. Без закрытого ключа новый ключ устанавливается через USB.
