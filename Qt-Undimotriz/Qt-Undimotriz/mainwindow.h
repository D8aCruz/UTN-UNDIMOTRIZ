#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include <QStandardPaths>
#include <QDesktopServices>
#include "pressureconverter.h"

// Forward declarations
class QPushButton;

#define ESPERO_INICIO_TRAMA      0
#define RECIBIENDO_DATOS         1
#define ESPERO_FIN_TRAMA         2

#define TRAMA_SIN_ESTADOS        6  // $[T1][T2][I1][I2]#
#define TRAMA_CON_ESTADOS        7  // $[T1][T2][I1][I2][ESTADOS]#

// Trama de 15 bytes (0xAA...0x55)
#define TRAMA_INICIO             0xAA
#define TRAMA_FIN                0x55
#define TRAMA_TOTAL_BYTES        15

// Índices de bytes en la nueva trama
#define IDX_INICIO               0
#define IDX_TENSION_MSB          1
#define IDX_TENSION_LSB          2
#define IDX_CORRIENTE_MSB        3
#define IDX_CORRIENTE_LSB        4
#define IDX_PRESION_MSB          5
#define IDX_PRESION_LSB          6
#define IDX_RPM_MSB              7
#define IDX_RPM_LSB              8
#define IDX_ACCEL_X_MSB          9
#define IDX_ACCEL_X_LSB          10
#define IDX_ACCEL_Y_MSB          11
#define IDX_ACCEL_Y_LSB          12
#define IDX_ESTADOS              13
#define IDX_FIN                  14

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool conectado();
    void actualizarEstadoConexion();
    bool parsearDireccionServidor(const QString &address, QString &host, quint16 &port);
    void procesarTramaTelemetria(); // Trama de 15 bytes con todos los sensores
    void procesarDatosRecibidos();
    void sincronizarEstadosCargas(uint8_t estadoDigital);
    void actualizarLCD();
    void enviarEstadosReles();  // Envía comando binario de 3 bytes con estado de todos los relés
    void actualizarEstadoBotonRele(QPushButton* boton, bool estado);
    void habilitarAreaMediciones(bool habilitar);
    void guardarMedicionesCSV(); // Guardar buffer de mediciones a CSV

    // Opciones para manejo cuando el usuario cancela el diálogo de guardar
    enum OpcionCancelacion {
        GUARDAR_PREDETERMINADO = 0,
        DESCARTAR_DATOS = 1,
        CANCELAR_VOLVER_GRABANDO = 2
    };


private slots:
    void on_datosRecibidos();
    void on_pushButtonConectar_clicked();
    void onSobretensionActivada();
    void onSobretensionDesactivada();
    // Slots para botones de cargas
    void on_pushButtonR1_clicked();
    void on_pushButtonR2_clicked();
    void on_pushButtonR3_clicked();
    void on_pushButtonR4_clicked();
    // Slot para iniciar/detener medición (toggle)
    void on_pushButtonMedicion_toggled(bool checked);

signals:
    void sobretensionActivada();
    void sobretensionDesactivada();
    void sobretensionCambiada(bool activa);

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
    QByteArray datosRecibidos;
    QTimer dataTimer;
    QTimer lcdTimer;
    PressureConverter pressureConverter;  // Gestor de conversión de presión

    // Variables para el protocolo de 402 bytes
    QByteArray tramaBuffer;
    int estadoRecepcion;
    int contadorBytes;


    // Variables para LCD y conversión (igual que ESP32)
    double tensionFiltrada;
    double corrienteFiltrada;
    double potenciaCalculada;
    bool datosValidos;

    // Sensores recibidos en la trama de 15 bytes
    double presionPa;
    double presionBar;
    double alturaFiltrada;      // Altura en cm
    uint16_t rpmActual;          // RPM (directo, sin conversión)
    double aceleracionX;         // Aceleración X en g
    double aceleracionY;         // Aceleración Y en g
    double anguloGrados;         // Angulo de apertura

    // Factores de sensibilidad para conversión ADC → físico
    static constexpr float FACTOR_SENSIBILIDAD_TENSION   = 0.1077f;   // V/cuenta
    static constexpr float FACTOR_SENSIBILIDAD_CORRIENTE = 0.00096; // A/cuenta
    static constexpr float FACTOR_SENSIBILIDAD_PRESION   = 1.0f;   // Pa/cuenta
    static constexpr float FACTOR_SENSIBILIDAD_ACCEL     = 1.0f; // g/cuenta

    // Estados de las cargas (false = desactivado, true = activado)
    // estadoRele0 corresponde a CARGA 1 (pushButtonR1)
    // estadoRele1 corresponde a CARGA 2 (pushButtonR2)
    // estadoRele2 corresponde a CARGA 3 (pushButtonR3)
    // estadoRele3 corresponde a CARGA 4 (pushButtonR4)
    bool estadoRele0;
    bool estadoRele1;
    bool estadoRele2;
    bool estadoRele3;
    bool alertaSobretensionActiva;

    // Control de sincronización inicial
    bool primeraTramaRecibida;
    bool medir;
    // Variables para captura de mediciones
    bool capturandoMediciones;
    QStringList datosMediciones;
    qint64 tiempoInicioMediciones;
};

#endif // MAINWINDOW_H
