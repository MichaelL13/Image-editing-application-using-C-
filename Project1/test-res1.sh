if [ ! -e p3150258-pizza1.c ]; then
    echo "file p3150258-pizza1.c not found"
    exit 1
fi

gcc -o main1 p3150258-pizza1.c -pthread

echo -e "Running with seed: 1000 and 100 customers ..."
./main1 1000 100
