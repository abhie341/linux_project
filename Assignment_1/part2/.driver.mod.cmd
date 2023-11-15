cmd_/home/abhi/22111401/part2/driver.mod := printf '%s\n'   driver.o | awk '!x[$$0]++ { print("/home/abhi/22111401/part2/"$$0) }' > /home/abhi/22111401/part2/driver.mod
