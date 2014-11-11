<?php


class MokiLizingas
{
    /** @var string Mokilizingas authorization username */
    protected $userName;

    /** @var string Mokilizingas authorization password */
    protected $userPassword;

    /** @var bool Payment processing mode */
    protected $testingMode;

    /** @var string Redirect URL to mokilizingas */
    protected $redirectUrl = 'https://secure.mokilizingas.lt/online/api/post';

    /** @var string Mokilizingas authorization URL */
    protected $authUrl = 'https://secure.mokilizingas.lt/online/api/auth';

    /** @var string Mokilizingas Payment status check URL */
    protected $checkUrl = 'https://secure.mokilizingas.lt/online/api/check';

    public function __construct($userName, $password, $testingMode = false)
    {
        $this->userName = trim($userName);
        $this->userPassword = trim($password);
        $this->testingMode = $testingMode;

    }

    public function startSession(MokilizingasOrder $order, $returnUrl, $alertUrl)
    {
        $sessionId = null;

        $orderData = $order->getOrderData();

        if ($this->testingMode) {
            $orderData['testing'] = 'Y';
        }

        $orderData['user_name'] = $this->userName;
        $orderData['user_psw'] = $this->userPassword;
        $orderData['return_url'] = $returnUrl;
        $orderData['alert_url'] = $alertUrl;

        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $this->getAuthUrl());
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
        curl_setopt($ch, CURLOPT_POST, true);
        curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($orderData));

        $result = curl_exec($ch);
        $resultData = json_decode($result);

        if (strlen($resultData->errorCode) > 0) {
            // remove user and password to prevent accidental display in error message for user!
            $orderData['user_name'] = '****';
            $orderData['user_psw'] = '****';

            throw new MokilizingasException($resultData->errorCode . ': ' . $resultData->errorMessage . "\n" . http_build_query($orderData));
        }

        return $resultData;
    }

    public function checkPayment($sessionId, $orderId)
    {

        $data = array(
            'session_id' => $sessionId,
            'order_id' => $orderId,
            'user_name' => $this->userName,
            'user_psw' => $this->userPassword
        );

        $ch = curl_init();

        curl_setopt($ch, CURLOPT_URL, $this->getPaymentCheckUrl());
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
        curl_setopt($ch, CURLOPT_POST, true);
        curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($data));

        $result = curl_exec($ch);
        $resultData = json_decode($result);


        if ($resultData->result == 'ERROR') {
            // remove user and password to prevent accidental display in error message for user!
            $orderData['user_name'] = '****';
            $orderData['user_psw'] = '****';

            throw new MokilizingasException($resultData->result. ': ' . $resultData->resultMsg . "\n" . http_build_query($orderData));
        }

        return $resultData;
    }

    /**
     * User redirect URL
     *
     * @return string
     */
    public function getRedirectUrl()
    {
        return $this->redirectUrl;
    }

    /**
     * Authorization URL
     *
     * @return string
     */
    public function getAuthUrl()
    {
        return $this->authUrl;
    }

    /**
     * Payment check URL
     * @return string
     */
    public function getPaymentCheckUrl()
    {
        return $this->checkUrl;
    }
}

class MokilizingasOrder
{
    protected $orderData;

    public function __construct($orderId, $orderAmount)
    {
        $this->orderData['order_id'] = $orderId;

        if (is_int($orderAmount) || is_float($orderAmount)) {
            $this->orderData['order_amount'] = $orderAmount;
        } else if (preg_match('/^([0-9]+)+([.,])+([0-9]{1,2})$/', $orderAmount) == 1 ) {
            $tmp = round(str_replace(',', '.', $orderAmount), 2);

            if ($tmp <= 0) {
                throw new MokilizingasException("Order amount should be more than 0. " . $orderAmount);
            }

            $this->orderData['order_amount'] = $orderAmount;
        } else {
            throw new MokilizingasException("Order amount should be a decimal. No thousand separator! " . $orderAmount);
        }
    }

    /**
     * Get order data
     *
     * @return array
     */
    public function getOrderData()
    {
        return $this->orderData;
    }

    /**
     * Set order information. Can be any additional information.
     *
     * @param $orderInfo
     */
    public function setOrderInfo($orderInfo)
    {
        $this->orderData['order_info'] = $orderInfo;
    }

    /**
     * Set client first name
     *
     * @param $firstName
     */
    public function setFirstName($firstName)
    {
        $this->orderData['first_name'] = $firstName;
    }

    /**
     * Set client last name
     *
     * @param $lastName
     */
    public function setLastName($lastName)
    {
        $this->orderData['last_name'] = $lastName;
    }

    /**
     * Set client phone number
     *
     * @param $clientPhone
     */
    public function setClientPhone($clientPhone)
    {
        $this->orderData['client_phone'] = $clientPhone;
    }

    /**
     * Set client email
     *
     * @param $clientEmail
     */
    public function setClientEmail($clientEmail)
    {
        $this->orderData['client_email'] = $clientEmail;
    }

    /**
     * Set client address
     *
     * @param $clientAddress
     */
    public function setClientAddress($clientAddress)
    {
        $this->orderData['client_address'] = $clientAddress;
    }

    /**
     * Set clients personal identification code
     *
     * @param $persCode
     */
    public function setPersCode($persCode)
    {
        $this->orderData['pers_code'] = $persCode;
    }

    /**
     * Set persons identification document type
     *
     * Availabel types:
     *  PP - pasport
     *  AK - personal identification card
     *
     * @param $docType
     */
    public function setDocType($docType)
    {
        if ($docType == 'PP' || $docType == 'AK') {
            $this->orderData['doc_type'] = $docType;
        } else {
            throw new MokilizingasException("Personal identification document type can only be 'PP' for passport and
            'AK' for personal identification card");
        }
    }

    /**
     * Set identification document number
     *
     * @param $docNo
     */
    public function setDocNo($docNo)
    {
        $this->orderData['doc_no'] = $docNo;
    }

    /**
     * Set vendor phone
     *
     * @param $vendorPhone
     */
    public function setVendorPhone($vendorPhone)
    {
        $this->orderData['vendor_phone'] = $vendorPhone;
    }

    /**
     * Set vendor email
     *
     * @param $vendorEmail
     */
    public function setVendorEmail($vendorEmail)
    {
        $this->orderData['vendor_email'] = $vendorEmail;
    }

    /**
     * Array in JSON format with shopping cart items
     * FORMAT
     * [
     *      {
     *          "name": "Item name", // string
     *          "category": "B", // string
     *          "price": "1123.56" // string
     *          "sale": false // boolean
     *      }
     * ]
     *
     * @param $orderJson
     */
    public function setOrderJson($orderJson)
    {
        $this->orderData['order_json'] = $orderJson;
    }
}

class MokilizingasException extends Exception {}