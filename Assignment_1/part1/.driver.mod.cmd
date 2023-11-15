cmd_/home/abhi/22111401/part1/driver.mod := printf '%s\n'   driver.o | awk '!x[$$0]++ { print("/home/abhi/22111401/part1/"$$0) }' > /home/abhi/22111401/part1/driver.mod
