#include <iostream>
#include <vector>
#include <string>
#include <algorithm>


class MyBigInt {
private:
    std::vector<unsigned int> data;

public:
    MyBigInt() {}

    void setHex(const std::string& hexString) {
        data.clear();
        // Додамо перевірку на ділимість довжини рядка на 8
        if (hexString.size() % 8 != 0) {
            std::cerr << "Invalid hex string length." << std::endl;
            return;
        }
        for (int i = hexString.size() - 8; i >= 0; i -= 8) {
            unsigned int chunk = std::stoul(hexString.substr(i, 8), 0, 16);
            data.push_back(chunk);
        }
    }

    std::string getHex() const {
        std::string hexString;
        for (int i = data.size() - 1; i >= 0; i--) {
            char chunkHex[9];
            std::snprintf(chunkHex, sizeof(chunkHex), "%08X", data[i]);
            hexString += chunkHex;
        }
        return hexString;
    }

    bool operator>=(const MyBigInt& other) const {
        if (data.size() > other.data.size()) {
            return true;
        }
        if (data.size() < other.data.size()) {
            return false;
        }
        for (int i = data.size() - 1; i >= 0; i--) {
            if (data[i] > other.data[i]) {
                return true;
            }
            if (data[i] < other.data[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator>(const MyBigInt& other) const {
        if (data.size() > other.data.size()) {
            return true;
        }
        if (data.size() < other.data.size()) {
            return false;
        }
        for (int i = data.size() - 1; i >= 0; i--) {
            if (data[i] > other.data[i]) {
                return true;
            }
            if (data[i] < other.data[i]) {
                return false;
            }
        }
        return false; // Якщо обидва числа рівні
    }

    MyBigInt(const char* hexString) {
        setHex(hexString);
    }

    MyBigInt XOR(const MyBigInt& other) const {
        MyBigInt result;
        size_t max_size = std::max(data.size(), other.data.size());
        result.data.resize(max_size, 0);
        unsigned int carry = 0;

        for (size_t i = 0; i < max_size; i++) {
            unsigned int a = (i < data.size()) ? data[i] : 0;
            unsigned int b = (i < other.data.size()) ? other.data[i] : 0;
            result.data[i] = a ^ b;
        }
        return result;
    }

    MyBigInt ADD(const MyBigInt& other) const {
        MyBigInt result;
        unsigned int carry = 0;

        size_t max_size = std::max(data.size(), other.data.size());
        result.data.resize(max_size, 0);

        for (size_t i = 0; i < max_size || carry; i++) {
            unsigned int a = (i < data.size()) ? data[i] : 0;
            unsigned int b = (i < other.data.size()) ? other.data[i] : 0;

            unsigned long long sum = static_cast<unsigned long long>(a) + b + carry;
            carry = static_cast<unsigned int>(sum >> 32);
            result.data[i] = static_cast<unsigned int>(sum);
        }

        if (carry) {
            // Якщо є перенос після останнього біта, додамо ще один біт.
            result.data.push_back(carry);
        }

        return result;
    }
    MyBigInt INV() const {
        MyBigInt result = *this;
        for (unsigned int& chunk : result.data) {
            chunk = ~chunk;
        }
        return result;
    }

    MyBigInt OR(const MyBigInt& other) const {
        MyBigInt result;
        size_t max_size = std::max(data.size(), other.data.size());
        result.data.resize(max_size, 0);

        for (size_t i = 0; i < max_size; i++) {
            unsigned int a = (i < data.size()) ? data[i] : 0;
            unsigned int b = (i < other.data.size()) ? other.data[i] : 0;
            result.data[i] = a | b;
        }
        return result;
    }

    MyBigInt AND(const MyBigInt& other) const {
        MyBigInt result;
        size_t max_size = std::max(data.size(), other.data.size());
        result.data.resize(max_size, 0);

        for (size_t i = 0; i < max_size; i++) {
            unsigned int a = (i < data.size()) ? data[i] : 0;
            unsigned int b = (i < other.data.size()) ? other.data[i] : 0;
            result.data[i] = a & b;
        }
        return result;
    }

    MyBigInt shiftR(int n) const {
        MyBigInt result = *this;
        int chunks_to_shift = static_cast<int>(n) / 32;
        int bits_to_shift = static_cast<int>(n) % 32;

        if (chunks_to_shift >= static_cast<int>(result.data.size())) {
            // Якщо кількість зсуву перевищує розмір числа, результат - нуль.
            result.data.assign(1, 0);
        }
        else if (chunks_to_shift > 0) {
            result.data.erase(result.data.begin(), result.data.begin() + chunks_to_shift);
        }

        if (bits_to_shift > 0) {
            unsigned int carry = 0;
            for (unsigned int& chunk : result.data) {
                unsigned int new_carry = chunk << (32 - bits_to_shift);
                chunk = (chunk >> bits_to_shift) | carry;
                carry = new_carry;
            }
        }

        return result;
    }

    MyBigInt shiftL(int n) const {
        MyBigInt result = *this;
        int chunks_to_shift = static_cast<int>(n) / 32;
        int bits_to_shift = static_cast<int>(n) % 32;

        if (chunks_to_shift > 0) {
            result.data.insert(result.data.begin(), chunks_to_shift, 0);
        }

        if (bits_to_shift > 0) {
            unsigned int carry = 0;
            for (int i = result.data.size() - 1; i >= 0; i--) {
                unsigned int& chunk = result.data[i];
                unsigned int new_carry = chunk >> (32 - bits_to_shift);
                chunk = (chunk << bits_to_shift) | carry;
                carry = new_carry;
            }
        }

        return result;
    }


    MyBigInt SUB(const MyBigInt& other) const {
        MyBigInt result;
        result.data.resize(std::max(data.size(), other.data.size()), 0);
        int borrow = 0;

        for (size_t i = 0; i < result.data.size(); i++) {
            int a = static_cast<int>((i < data.size()) ? data[i] : 0);
            int b = static_cast<int>((i < other.data.size()) ? other.data[i] : 0);
            int diff = a - b - borrow;
            borrow = (diff < 0) ? 1 : 0;
            result.data[i] = static_cast<unsigned int>((diff + (1 << 31)) & 0xFFFFFFFF);
        }

        // Видаляємо непотрібні нульові чанки з початку.
        while (!result.data.empty() && result.data.back() == 0) {
            result.data.pop_back();
        }

        return result;
    }

    MyBigInt MOD(const MyBigInt& divisor) const {
        MyBigInt dividend = *this;
        MyBigInt quotient, remainder;

        while (dividend >= divisor) {
            MyBigInt tempDivisor = divisor;
            MyBigInt tempQuotient;
            int shiftAmount = static_cast<int>(dividend.data.size()) - static_cast<int>(tempDivisor.data.size());
            tempDivisor = tempDivisor.shiftL(shiftAmount);

            while (dividend >= tempDivisor) {
                dividend = dividend.SUB(tempDivisor);
                tempQuotient = tempQuotient.ADD(MyBigInt("1"));
            }

            int shiftAmount2 = static_cast<int>(dividend.data.size()) - static_cast<int>(tempDivisor.data.size());
            tempQuotient = tempQuotient.shiftR(shiftAmount2);
            quotient = quotient.ADD(tempQuotient);
        }

        remainder = dividend;
        return remainder;
    }

    MyBigInt MUL(const MyBigInt& other) const {
        MyBigInt result;

        for (size_t i = 0; i < data.size(); i++) {
            unsigned int carry = 0;
            MyBigInt tempResult;
            tempResult.data.resize(i, 0);

            for (size_t j = 0; j < other.data.size() || carry; j++) {
                unsigned long long current =
                    static_cast<unsigned long long>(data[i]) * (j < other.data.size() ? other.data[j] : 0) + carry;
                tempResult.data.push_back(static_cast<unsigned int>(current & 0xFFFFFFFF));
                carry = static_cast<unsigned int>(current >> 32);
            }

            result = result.ADD(tempResult);
        }

        return result;
    }

    MyBigInt DIV(const MyBigInt& divisor) const {
        MyBigInt quotient, remainder, tempDividend = *this;

        while (tempDividend >= divisor) {
            MyBigInt tempDivisor = divisor;
            MyBigInt tempQuotient;
            int shiftAmount = static_cast<int>(tempDividend.data.size()) - static_cast<int>(tempDivisor.data.size());
            tempDivisor = tempDivisor.shiftL(shiftAmount);

            while (tempDividend >= tempDivisor) {
                tempDividend = tempDividend.SUB(tempDivisor);
                tempQuotient = tempQuotient.ADD(MyBigInt("1"));
            }

            int shiftAmount2 = static_cast<int>(tempDividend.data.size()) - static_cast<int>(tempDivisor.data.size());
            tempQuotient = tempQuotient.shiftR(shiftAmount2);
            quotient = quotient.ADD(tempQuotient);
        }

        remainder = tempDividend;
        return remainder;
    }

};

int main() {
    MyBigInt numberA, numberB, result;

    numberA.setHex("36f028580bb02cc8272a9a020f4200e346e276ae664e45ee80745574e2f5ab80");
    numberB.setHex("70983d692f648185febe6d6fa607630ae68649f7e6fc45b94680096c06e4fadb");

    result = numberA.XOR(numberB);
    std::cout << "XOR Result: " << result.getHex() << std::endl;

    result = numberA.ADD(numberB);
    std::cout << "ADD Result: " << result.getHex() << std::endl;

    MyBigInt invResult = numberA.INV();
    std::cout << "INV Result: " << invResult.getHex() << std::endl;

    MyBigInt orResult = numberA.OR(numberB);
    std::cout << "OR Result: " << orResult.getHex() << std::endl;

    MyBigInt andResult = numberA.AND(numberB);
    std::cout << "AND Result: " << andResult.getHex() << std::endl;

    MyBigInt shiftRightResult = numberA.shiftR(16);
    std::cout << "Shift Right Result: " << shiftRightResult.getHex() << std::endl;

    MyBigInt shiftLeftResult = numberA.shiftL(16);
    std::cout << "Shift Left Result: " << shiftLeftResult.getHex() << std::endl;

    MyBigInt subresult = numberA.SUB(numberB);
    std::cout << "SUB Result: " << subresult.getHex() << std::endl;

    MyBigInt modResult = numberA.MOD(numberB);
    std::cout << "MOD Result: " << modResult.getHex() << std::endl;

    MyBigInt mulResult = numberA.MUL(numberB);
    std::cout << "MUL Result: " << mulResult.getHex() << std::endl;

    MyBigInt divResult = numberA.DIV(numberB);
    std::cout << "DIV Result: " << divResult.getHex() << std::endl;
    return 0;
}

