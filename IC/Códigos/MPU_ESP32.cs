using System;
using System.Collections.Generic;
using UnityEngine;
using System.Text;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;

public class MPU_ESP32 : MonoBehaviour
{
    private List<string> eventMessages = new List<string>();

    private Queue<string> messageQueue = new Queue<string>();

    private bool autoTest = false;
    private bool updateUI = false;
    private Rigidbody rb; // Referência ao Rigidbody

    private MqttClient client;
    private string brokerAddress = "localhost";

    private string topic = "esp_Envia";

    void Start()
    {
        rb = GetComponent<Rigidbody>(); // Obtém o Rigidbody no Start

        client = new MqttClient(brokerAddress);

        try
        {
            client.Connect("unity_client");
            Debug.Log("✅ Conectado ao broker MQTT!");
        }
        catch (Exception e)
        {
            Debug.LogError($"❌ Falha ao conectar ao MQTT! Erro: {e.Message}");
        }

        client.Subscribe(new string[] { topic }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE });client.MqttMsgPublishReceived += OnMqttMessageReceived;
        client.MqttMsgPublishReceived += OnMqttMessageReceived;

        if (rb == null)
        {
            Debug.LogError("Rigidbody não encontrado! Adicione um Rigidbody ao GameObject.");
        }
    }

    void OnMqttMessageReceived(object sender, MqttMsgPublishEventArgs e)
    {
        string msg = Encoding.UTF8.GetString(e.Message);
        Debug.Log($"📡 Mensagem recebida via MQTT: {msg}"); // 🔴 Mostra os dados no console do Unity

        lock (messageQueue)
        {
            messageQueue.Enqueue(msg);
        }
    }


    protected virtual void DecodeMessage(string topic, byte[] message)
    {
        string msg = Encoding.UTF8.GetString(message);
        Debug.Log("Received: " + msg);
        StoreMessage(msg);

        if (topic == "M2MQTT_Unity/test")
        {
            if (autoTest)
            {
                autoTest = false;
                Disconnect();
            }
        }
    }

    void StoreMessage(string eventMsg)
    {
        eventMessages.Add(eventMsg);
    }

    void ProcessMessage(string msg)
    {
        AddUiMessage("Received: " + msg);

        // Aplica a movimentação e rotação com base nos dados do sensor
        ApplyMotionAndRotation(msg);
    }

    void ApplyMotionAndRotation(string msg)
{
    if (rb == null) return;

    string[] values = msg.Split(',');
    if (values.Length < 6) return;

    if (float.TryParse(values[0], out float ax) &&
        float.TryParse(values[1], out float ay) &&
        float.TryParse(values[2], out float az) &&
        float.TryParse(values[3], out float gx) &&
        float.TryParse(values[4], out float gy) &&
        float.TryParse(values[5], out float gz))
    {
        // Multiplicadores para tornar o movimento visível
        float acelMult = 0.01f;
        float rotMult = 0.25f;

        // Zera movimento anterior (opcional)
        rb.linearVelocity = Vector3.zero;
        rb.angularVelocity = Vector3.zero;

        // Aplica força como aceleração
        Vector3 acceleration = new Vector3(ax, ay, az) * acelMult;
        rb.AddForce(acceleration, ForceMode.Impulse); // Troquei para Impulse para impacto imediato

        // Aplica rotação
        Vector3 angularVelocity = new Vector3(-gx, -gy, gz) * rotMult;
        rb.angularVelocity = angularVelocity;

        Debug.DrawRay(rb.position, acceleration, Color.green, 1f);

        Debug.Log($"🎯 Movimento aplicado:\nForça: {acceleration}\nRotação: {angularVelocity}");

        
        // Espera um tempo e zera a velocidade
        StartCoroutine(ResetMovementAfterDelay(0.2f));
    }
    else
    {
        Debug.LogWarning("⚠️ Erro ao converter valores da mensagem.");
    }
}

System.Collections.IEnumerator ResetMovementAfterDelay(float delay)
{
    yield return new WaitForSeconds(delay);
    rb.linearVelocity = Vector3.zero;
    rb.angularVelocity = Vector3.zero;
    Debug.Log("🛑 Movimento zerado após impulso.");
}


    protected void Update()
    {
        ProcessMqttEvents();
        if (Input.GetKeyDown(KeyCode.T))
        {
            rb.linearVelocity = Vector3.zero;
            rb.angularVelocity = Vector3.zero;

            // Altere a posição e rotação como quiser
            rb.transform.position = new Vector3(0f, 1f, 0f); // posição inicial (levemente elevada)
            rb.transform.rotation = Quaternion.identity; // rotação zerada

            Debug.Log("📍 Posição e rotação resetadas!");
        }

        if (Input.GetKeyDown(KeyCode.Space))
        {
            rb.linearVelocity = Vector3.zero;
            rb.angularVelocity = Vector3.zero;
            rb.AddForce(new Vector3(0, 0, 3f), ForceMode.Impulse);
            Debug.Log("🔥 Força manual aplicada");
        }

        lock (messageQueue)
        {
            while (messageQueue.Count > 0)
            {
                string msg = messageQueue.Dequeue();
                ProcessMessage(msg);
            }
        }

        if (updateUI)
        {
            UpdateUI();
        }
    }

    void ProcessMqttEvents()
    {
        // Implementação da lógica de eventos MQTT, se necessário
    }

    void AddUiMessage(string message)
    {
        Debug.Log("UI: " + message);
    }

    void Disconnect()
    {
        Debug.Log("MQTT Disconnected");
    }

    void UpdateUI()
    {
        Debug.Log("UI Updated");
    }
}