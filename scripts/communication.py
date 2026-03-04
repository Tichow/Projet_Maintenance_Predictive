"""
script de communication UART avec la carte pour tester l'inference du modele

le protocole est simple, on envoie d'abord 0xAB au microcontroleur,
puis on attend 0xCD en retour (la carte est prete),
enfin on boucle sur les donnees de test :
     - envoi de 8 float32 (32 octets)
     - reception de 5 uint8 (probabilites des 5 classes)
"""

import serial
import numpy as np
import sys
import time

#config

PORT = 'COM3'
BAUDRATE = 115200
NB_INPUTS = 8
NB_OUTPUTS = 5
SYNC_BYTE = 0xAB
ACK_BYTE = 0xCD

CLASS_NAMES = ['Functional', 'TWF', 'HDF', 'PWF', 'OSF']


def uart_sync(ser):
    """envoie 0xAB, attend 0xCD en retour"""
    print("synchronisation avec la STM32...")
    ser.write(bytes([SYNC_BYTE]))
    response = ser.read(1)
    if len(response) == 0 or response[0] != ACK_BYTE:
        print("ERREUR : pas de reponse de la STM32")
        print(f"  - verifier le port {PORT}")
        print("  - verifier que le firmware est flashe")
        print("  - fermer tout autre programme sur le port serie")
        sys.exit(1)
    print("synchronisation ok")


def send_inputs(ser, inputs):
    """envoie 8 float32 (32 octets) a la STM32"""
    data = inputs.astype(np.float32).tobytes()
    ser.write(data)


def read_outputs(ser):
    """lit 5 octets et reconvertit en probas [0.0 - 1.0]"""
    response = ser.read(NB_OUTPUTS)
    if len(response) != NB_OUTPUTS:
        return None
    return np.array([b / 255.0 for b in response])


def main():
    # chargement des donnees de test (generees par le notebook)
    X_test = np.load('modele/x_test.npy')
    Y_test = np.load('modele/y_test.npy')
    print(f"donnees chargees : {X_test.shape[0]} echantillons, {X_test.shape[1]} features")

    # connexion serie
    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=10)
        time.sleep(2)  # laisser le temps a la connexion de s'etablir
        print(f"connecte sur {PORT} a {BAUDRATE} baud")
    except serial.SerialException as e:
        print(f"erreur de connexion : {e}")
        sys.exit(1)

    # synchronisation
    uart_sync(ser)

    # boucle de test
    correct = 0
    total = 0
    uart_errors = 0
    nb_tests = len(X_test)

    for i in range(nb_tests):
        send_inputs(ser, X_test[i])
        probas = read_outputs(ser)

        if probas is None:
            uart_errors += 1
            continue

        predicted = np.argmax(probas)
        expected = np.argmax(Y_test[i])

        if predicted == expected:
            correct += 1
        total += 1

        # affichage tous les 200 echantillons
        if (i + 1) % 200 == 0:
            acc = correct / total * 100 if total > 0 else 0
            print(f"  [{i+1}/{nb_tests}] accuracy : {acc:.1f}%")

    # resultats
    acc = correct / total * 100 if total > 0 else 0
    print(f"\n{'='*50}")
    print(f"RESULTATS - INFERENCE SUR STM32L4R9")
    print(f"{'='*50}")
    print(f"echantillons testes    : {total}")
    print(f"predictions correctes  : {correct}")
    print(f"accuracy sur cible     : {acc:.1f}%")
    print(f"erreurs UART           : {uart_errors}")
    print(f"{'='*50}")

    ser.close()


if __name__ == '__main__':
    main()
